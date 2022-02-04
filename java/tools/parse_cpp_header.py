"""
Use clang to parse C++ header and produce a hierarchical view of the types within it.

Each node in the tree is an instance of the Scope class.

You must have both the clang python package and the clang (llvm version 11 or later recommended) compiler installed.
"""

import os
import sys
import platform
from typing import Tuple, List, Union
import re
import fnmatch
import dataclasses
from dataclasses import dataclass

from os import name, path, stat
from clang import cindex
from clang.cindex import Cursor, CursorKind, Type
from collections import OrderedDict
from glob import glob

class NoFilenamesError(ValueError):
    pass

class UnknownOptionError(ValueError):
    pass

# From Python 3.7 dict insertion order is guaranteed
assert sys.version_info.major >= 3 and sys.version_info.minor >= 7 

# If these asserts don't hold, the canonical type remapping must be reworked.
assert cindex.sizeof(cindex.c_long) == 4         
assert cindex.sizeof(cindex.c_longlong) == 8
assert cindex.sizeof(cindex.c_longdouble) == 8

_remap_canonical_types = {
    'unsigned long long': 'uint64_t',
    'long long':          'int64_t',
    'long double':        'double',
    'unsigned long':      'uint32_t',
    'long':               'int32_t',
}

compiler_include_directories = []

RECURSE_LIST = [
    CursorKind.TRANSLATION_UNIT,
    CursorKind.NAMESPACE,
    CursorKind.CLASS_DECL,
    CursorKind.CLASS_TEMPLATE,
    CursorKind.STRUCT_DECL,
    CursorKind.ENUM_DECL,
    CursorKind.CXX_METHOD,
    CursorKind.FUNCTION_DECL,
    CursorKind.FUNCTION_TEMPLATE,
    CursorKind.CONSTRUCTOR,
    CursorKind.TYPE_ALIAS_DECL,
    CursorKind.TYPE_REF,
    CursorKind.TEMPLATE_REF,
]

PRINT_LIST = [
    CursorKind.NAMESPACE,
    CursorKind.CLASS_DECL,
    CursorKind.STRUCT_DECL,
    CursorKind.ENUM_DECL,
    CursorKind.ENUM_CONSTANT_DECL,
    CursorKind.FUNCTION_DECL,
    CursorKind.FUNCTION_TEMPLATE,
    CursorKind.CONVERSION_FUNCTION,
    CursorKind.CXX_METHOD,
    CursorKind.CXX_BASE_SPECIFIER,
    CursorKind.CONSTRUCTOR,
    CursorKind.DESTRUCTOR,
    CursorKind.FIELD_DECL,
    CursorKind.PARM_DECL,
    CursorKind.TYPE_ALIAS_DECL,
    CursorKind.CLASS_TEMPLATE,
    CursorKind.TYPE_REF,
    CursorKind.TEMPLATE_REF,
]

PREFIX_BLACKLIST = [
    CursorKind.TRANSLATION_UNIT
]

PUBLIC_LIST = [
  cindex.AccessSpecifier.INVALID,
  cindex.AccessSpecifier.PUBLIC
]

LITERAL_TYPES = [ 
    CursorKind.CHARACTER_LITERAL, 
    CursorKind.COMPOUND_LITERAL_EXPR, 
    CursorKind.CXX_BOOL_LITERAL_EXPR, 
    CursorKind.CXX_NULL_PTR_LITERAL_EXPR, 
    CursorKind.FLOATING_LITERAL, 
    CursorKind.IMAGINARY_LITERAL, 
    CursorKind.INTEGER_LITERAL, 
    CursorKind.OBJC_STRING_LITERAL, 
    CursorKind.OBJ_BOOL_LITERAL_EXPR, 
    CursorKind.STRING_LITERAL 
]

_file_contents_dict = {}

class ScopeDoc:
    def __init__(self, summary: str='', returns: str='', see: str=''):
        self.fields = {}
        if summary:
            self.summary    = summary
        if returns: 
            self.returns    = returns
        if see: 
            self.see        = see
        self._params    = OrderedDict()

    def __bool__(self) -> bool:
        return bool(self.fields) or bool(self._params)

    def __repr__(self) -> str:
        return self.summary

    @property
    def summary(self) -> str:
        if 'summary' in self.fields:
            return self.fields['summary']
        else:
            return ''

    @summary.setter
    def summary(self, value):
        self.fields['summary'] = value

    @property
    def returns(self) -> str:
        if 'returns' in self.fields:
            return self.fields['returns']
        else:
            return ''

    @returns.setter
    def returns(self, value):
        self.fields['returns'] = value

    @property
    def see(self) -> str:
        if 'see' in self.fields:
            return self.fields['see']
        else:
            return ''

    @see.setter
    def see(self, value):
        self.fields['see'] = value

    @property
    def params(self) -> OrderedDict:
        return self._params

    def add_param(self, name: str, docstring: str):
        self._params[name] = docstring

    @staticmethod
    def parse_comment(text: str) -> 'ScopeDoc':
        import re
        w = re.compile(r'[a-zA-Z_][a-zA-Z0-9_]*')
        text = text.strip()
        if text.startswith('///<'):
            return ScopeDoc(summary=text.replace('///<', '').strip())
        elif text.startswith('/// \\'):
            doc = ScopeDoc()
            lines = [ l.replace('/// ', '').strip() for l in text.splitlines() ]
            summary = []
            for l in lines:
                if l.startswith('\\param '):
                    l = l[7:].strip()
                    name = w.match(l)[0]
                    txt = l[len(name):].strip()
                    doc.add_param(name, txt)
                elif l.startswith('\\class '):
                    pass
                elif l.startswith('\\brief '):
                    summary.append(l[7:].strip())
                elif l.startswith('\\return '):
                    doc.returns = l[8:].strip()
                elif l.startswith('\\see '):
                    doc.see = l[4:].strip()
                elif l.startswith('\\'):
                    pass
                else:
                    summary.append(l)
            if summary:
                doc.summary = '\n'.join(summary)
            return doc
        elif text.startswith('/// <'):
            lines = [ l.replace('/// ', '').strip() for l in text.splitlines() ]
            import xml.etree.ElementTree as ET
            xmlstring = '<doc>' + '\n'.join(lines) + '</doc>'
            try:
                tree = ET.ElementTree(ET.fromstring(xmlstring))
                root = tree.getroot()
                it = root.getiterator()
                next(it) # skip root
                doc = ScopeDoc()
                for n in it:
                    if n.tag == 'param':
                        name = n.attrib['name']
                        txt = n.text.strip() if n.text else ''
                        doc.add_param(name, txt)
                    else:
                        doc.fields[n.tag] = n.text.strip() if n.text else ''
                return doc
            except Exception as e:
                print(f'ERROR IN DOCSTRING: {xmlstring}')
                return ScopeDoc()
        else:
            return ScopeDoc()

def _get_namespace(typename: str) -> str:
    if '::' in typename:
        ns = '::'.join(typename.split('::')[:-1])
        assert ns
    else:
        ns = ''
    return ns

def _ensure_namespaced(typename: str, default_namespace: str) -> str:
    if '::' in typename:
        return typename
    elif default_namespace:
        return f'{default_namespace}::{typename}'
    else:
        return typename

class Param:
    def __init__(self, typename: str = '', name: str = '', canonical_type: str = '', default_value: str ='', comment: str ='', is_out: bool = False,  node: Cursor=None, default_namespace=''):
        assert isinstance(typename, str)
        self.typename       = ''
        self.name           = ''
        self.canonical_type = ''
        self.comment        = ''
        self.default_value  = ''
        self.typenode       = None
        self.is_out         = is_out
        if node:
            if node.kind == CursorKind.PARM_DECL:
                self.name           = node.spelling
                self.typenode       = node.type
                self.default_value  = Param.get_default_value(node)
                self.comment        = node.raw_comment
            else:
                self.typenode = node
            assert isinstance(self.typenode, Type)
            self.typename       = typename or self.typenode.spelling
            self.canonical_type = canonical_type or Scope.type_get_canonical_name(self.typenode)
            if '<' in self.canonical_type:
              # Local typedefs are not namespaced, even when get_canonical(), so we may have to jump through some hoops to get there
              self.canonical_type = _ensure_namespaced(self.canonical_type, _get_namespace(self.typename))
              self.canonical_type = _ensure_namespaced(self.canonical_type, default_namespace)
        self.typename       = typename or self.typename
        self.name           = name or self.name
        self.canonical_type = canonical_type or typename or self.canonical_type
        self.comment        = comment or self.comment
        self.default_value  = default_value or self.default_value
        self.canonical_type = Scope.typename_get_canonical_name(self.canonical_type)
        assert 'long long' not in self.canonical_type

    @staticmethod
    def get_default_value(node: Cursor) -> str:
        if node.kind == CursorKind.PARM_DECL:
            c = [*node.get_children()]
            if c and [item.kind for item in c if item.kind in LITERAL_TYPES]:
                assert len(c) == 1, "fixme!"
                item = c[0]
                tokens = " ".join([token.spelling for token in item.get_tokens()])
                return tokens
        return ''

class Scope(OrderedDict):
    def __init__(self, parent, nodetype, name, prefix, typename, kind, node, overload_name = None):
        super().__init__()
        self.parent         = parent
        self.nodetype       = nodetype
        self.name           = name
        self._prefix        = prefix # SteinFIXME remove ?
        self.typename       = typename
        self.kind           = kind
        self.node           = node
        self.overload_name  = overload_name or name
        self._comment       = None

    def __bool__(self) -> bool: # SteinFixme. Refactor to not use OrderedDict at all
        return self is not None

    def __repr__(self):
        nodetype = self.nodetype
        name     = self.name
        typename = self.typename
        kind     = self.kind
        if self.is_static_method:
            typename = "static " + typename
        return f"{nodetype} {name} {typename} {kind}"

    def get_doc(self):
        return ScopeDoc.parse_comment(self.comment)

    def dump(self, indent="", show_comments=False, file=sys.stdout):
        childindent = indent + "    " if self.name else indent
        if show_comments and self.node and self.comment:
            comments = [ indent + comment.strip() for comment in self.comment.splitlines() ]
            comment = '\n'.join(comments)
            print(f'\n{comment}', file=file)
        if self.name:
            print(indent + str(self), file=file)
        for child in self.keys():
            self[child].dump(indent=childindent, show_comments=show_comments, file=file)
            
    def find(self, scope: str) -> 'Scope':
        scope = scope.split('<')[0]
        tokens = scope.split('::')
        if len(tokens) > 1:
            return self[tokens[0]].find("::".join(tokens[1:]))
        else:
            return self[scope]

    def try_find(self, scope: str) -> 'Scope':
        try:
            return self.find(scope)
        except KeyError:
            return None
            
    def get_children(self, nodetype_filter = []) -> List['Scope']:
        children = []
        for child in self.keys():
            if not nodetype_filter or self[child].nodetype in nodetype_filter:
                children.append(self[child])
        return children

    @staticmethod
    def _get_comment_from_file(filename: str, lineno: int):
        assert lineno > 0
        lineno = lineno - 1
        global _file_contents_dict
        if not filename in _file_contents_dict.keys():
            try:
                with open(filename) as file:
                    contents = file.read().splitlines()
                    _file_contents_dict[filename] = contents
            except:
                _file_contents_dict[filename] = []
        contents = _file_contents_dict[filename]
        if lineno < len(contents):
            line = contents[lineno]
            try:
                return line[line.index('///'):]
            except:
                pass
        return ''

    @staticmethod
    def _get_comment(node) -> str:
        comment = node.raw_comment or Scope._get_comment_from_file(node.location.file.name, node.location.line)
        return comment

    @property 
    def comment(self) -> str:
        if self._comment is None:
            if self.node: # and self.is_function or self.is_enum_value or self.is_record:
                self._comment = Scope._get_comment(self.node)
            else:
                self._comment = ''
        return self._comment

    @property
    def root(self):
        if self.parent:
            return self.parent.root
        else:
            return self

    @property
    def filename(self) -> str:
        return self.node.location.file.name

    @staticmethod
    def get_node_prefix(node: Union[Cursor, Type]) -> str:
        if isinstance(node, Cursor):
            node = node.get_definition() or node
        else:
            node = node.get_declaration() or node
        assert isinstance(node, Cursor)
        pr = []
        while node.semantic_parent and node.semantic_parent.semantic_parent: # Root is translation unit node, and we don't want that
            node = node.semantic_parent
            pr.insert(0, node.spelling)
        return '::'.join(pr)

    @staticmethod
    def get_node_fullname(node: Union[Cursor, Type], override_name='') -> str:
        name = override_name or d(node.spelling)
        prefix = Scope.get_node_prefix(node)
        if prefix:
            return f'{prefix}::{name}'
        else:
            return name

    @property
    def prefix(self) -> str:
        assert self.node
        return Scope.get_node_prefix(self.node)

    @property
    def fullname(self) -> str: 
        return Scope.get_node_fullname(self.node, self.name)

    @property
    def is_enum(self) -> bool:
        return self.nodetype == 'ENUM_DECL'

    @property
    def is_enum_value(self) -> bool:
        return self.nodetype == 'ENUM_CONSTANT_DECL'
    
    @property
    def enum_integral_type(self) -> str:
        assert self.is_enum
        return self.node.enum_type.spelling

    def get_enum_values(self) -> List[Tuple[str, int, str]]: # name, value, docstring
        values = [(c.name, c.enum_value, str(c.get_doc())) for c in self.get_children() if c.is_enum_value]
        return values

    def get_unique_enum_values(self) -> List[Tuple[str, int, str]]: # name, value, docstring
        test = set()
        values = self.get_enum_values()
        unique_values = []
        for n,v,d in values:
            if v not in test:
                test.add(v)
                unique_values.append((n, v, d))
        return unique_values

    @property    
    def enum_value(self) -> int:
        return self.node.enum_value        

    @property
    def is_function(self) -> bool:
        return self.kind == 'FUNCTIONPROTO'

    @property
    def is_template(self) -> bool:
        return self.nodetype == 'CLASS_TEMPLATE' or self.nodetype == 'FUNCTION_TEMPLATE'

    @property 
    def is_class_method(self) -> bool:
        return self.nodetype == 'CXX_METHOD'
        
    @property
    def is_record(self) -> bool:
        "struct or class"
        return self.kind == 'RECORD'

    @property
    def is_namespace(self) -> bool:
        "namespace"
        return self.kind == "NAMESPACE"
        
    @property 
    def is_constructor(self) -> bool:
        return self.nodetype == 'CONSTRUCTOR'

    @property
    def is_default_constructor(self) -> bool:
        return self.node.is_default_constructor()

    @property
    def is_copy_constructor(self) -> bool:
        return self.node.is_copy_constructor()

    @property
    def is_move_constructor(self) -> bool:
        return self.node.is_move_constructor()

    @property
    def is_converting_constructor(self) -> bool:
        return self.node.is_converting_constructor()

    @property
    def is_destructor(self) -> bool:
        return self.is_function and self.name.startswith("~")

    @property
    def is_abstract(self) -> bool:
        return any([c.is_pure_virtual_method for c in self.get_children()])

    @property
    def is_pure_virtual_method(self) -> bool:
        return self.node.is_pure_virtual_method()

    @property
    def is_anonymous(self) -> bool:
        return self.node.is_anonymous()

    @staticmethod
    def typename_get_canonical_name(name: str) -> str:
        for canonical in _remap_canonical_types:
            if canonical in name:
                name = name.replace(canonical, _remap_canonical_types[canonical])
        return name

    @staticmethod
    def type_get_canonical_name(node: Type) -> str:
        name = node.get_canonical().spelling
        return Scope.typename_get_canonical_name(name)

    def get_base_classes(self) -> List[Tuple['Scope', str]] :
        """Returns a list of 2-tuples where the first is the class node and the second is either an empty string OR a list of template parameters in the form '<type-parameter-0-0, ...>"""
        assert self.node
        def get_definition(node: Cursor):
            if node.kind == CursorKind.TYPE_ALIAS_DECL:
                alias_to = next(node.get_children())
                assert alias_to.kind == CursorKind.TEMPLATE_REF or alias_to.kind == CursorKind.TYPE_REF
                return alias_to.get_definition()
            else:
                return node

        def get_template_args(node: Cursor) -> str:
            name = Scope.type_get_canonical_name(node.type)
            if '<' in name and '>' in name:
                name = Scope.typename_get_canonical_name(name[name.index('<') : name.rindex('>') + 1])
                assert 'long long' not in name
                return name
            else:
                return ''

        parentnode = get_definition(self.node)
        basenodes = [node for node in parentnode.get_children() if node.kind == CursorKind.CXX_BASE_SPECIFIER]
        bases = [(self.root.find(node.type.spelling), '') for node in basenodes if self.root.try_find(node.type.spelling)]
        if basenodes and not bases:
            bases = [(self.root.find(Scope.get_node_fullname(node.type.get_canonical())), get_template_args(node)) for node in basenodes if self.root.try_find(Scope.get_node_fullname(node.type.get_canonical()))]
        return bases

    @property
    def base_class(self) -> Tuple['Scope', str]:
        """Returns None or a 2-tuple where the first is the class node and the second is either an empty string OR a list of template parameters in the form '<type-parameter-0-0, ...>"""
        bases = self.get_base_classes()
        return bases[0] if bases else None

    def get_args(self) -> List[Param]:
        node = self.node
        default_namespace = self.parent.prefix
        paramnodes = [n for n in node.get_children() if n.kind == CursorKind.PARM_DECL]
        params = [Param(node=n, default_namespace=default_namespace) for n in paramnodes]
        return params
        
    @property
    def is_static_method(self) -> bool:
        if hasattr(self.node, "is_static_method"):
            return self.node.is_static_method()
        return False

    @property
    def result(self) -> Param:
        if self.is_function:
            default_namespace = self.parent.prefix
            return Param(is_out=True, name='result', node=self.node.result_type, default_namespace = default_namespace)

    @property
    def result_type(self) -> str:
        if self.kind == "FUNCTIONPROTO":
            return Scope.type_get_canonical_name(self.node.result_type)
        else:
            return self.typename

    @property
    def is_operator(self) -> bool:
        if self.name.startswith("operator"):
            return True
        else:
            return False

    @property
    def is_typealias(self) -> bool:
        return self.nodetype == 'TYPE_ALIAS_DECL'

    @property
    def is_explicit_instantiation(self) -> bool:
        c = self.get_children()
        if len(c) >= 1:
            if c[0].nodetype == 'TEMPLATE_REF':
                return True
        return False

    def create_overload_name(self, node) -> str:
        name = d(node.spelling)
        if node.kind in [
            CursorKind.FUNCTION_DECL,
            CursorKind.FUNCTION_TEMPLATE,
            CursorKind.CONVERSION_FUNCTION,
            CursorKind.CXX_METHOD,
            CursorKind.CONSTRUCTOR,
            CursorKind.DESTRUCTOR,
        ] :
            instances = [item for item in self.keys() if self[item].name == name]
            name = name if not instances else name + str(len(instances) + 1)
        return name

def d(s):
    return s if isinstance(s, str) else s.decode('utf8')
    
def q(s):
    return '"' + s + '"'

def recurse_nodes(filename: str, node: cindex.Cursor, container: Scope, prefix: str =''):
    if not node.access_specifier in PUBLIC_LIST:
        return
    parent_container = container
    try:
        node.kind
    except Exception as e:
        print('\n*** Unknown node type. You may need to upgrade your clang bindings ***', file=sys.stderr)
        print(e, file=sys.stderr)
        print('', file=sys.stderr)
        return
    if node.kind in PRINT_LIST:
        name = d(node.spelling)
        uname = parent_container.create_overload_name(node)
        nodetype = str(node.kind).replace("CursorKind.", "")
        if node.kind == CursorKind.NAMESPACE:
            typename = "NAMESPACE"
            kind = "NAMESPACE"
        else:
            typename = Scope.type_get_canonical_name(node.type)
            kind = str(node.type.get_canonical().kind).replace("TypeKind.", "")
        if (kind == 'RECORD' or nodetype == 'ENUM_DECL') and len(list(node.get_children())) == 0:
            return
        if uname in container:
            #assert container[uname].kind == kind
            parent_container = container[uname]
        else:
            parent_container = Scope(container, nodetype, name, prefix, typename, kind, node, uname)
            container[uname] = parent_container
    if node.kind in RECURSE_LIST:
        sub_prefix = prefix
        if not node.kind in PREFIX_BLACKLIST:
            if len(sub_prefix) > 0:
                sub_prefix += '::'
            sub_prefix += d(node.spelling)
        for child in node.get_children():
            recurse_nodes(filename, child, parent_container, sub_prefix)

def parse_file(filename: str, parameters: List[str], log_to=sys.stderr) -> Scope:
#    print(f"Processing '{filename}'", file=log_to)
    index = cindex.Index(cindex.conf.lib.clang_createIndex(False, True))
    tu = index.parse(filename, parameters)
    root = Scope(None, "TRANSLATION_UNIT", os.path.abspath(filename), "", "", "", tu)
    recurse_nodes(filename, tu.cursor, root)
    return root

def parse_args(args: List[str]) -> Tuple[List[str], List[str]]:
    parameters = ['-DJAVA_WRAPPER_GENERATOR']
    filenames = []
    if "-x" not in args:
        parameters.extend(['-x', 'c++'])
    if platform.system() == 'Darwin':
        dev_path = '/Applications/Xcode.app/Contents/Developer/'
        lib_dir = dev_path + 'Toolchains/XcodeDefault.xctoolchain/usr/lib/'
        sdk_dir = dev_path + 'Platforms/MacOSX.platform/Developer/SDKs'
        libclang = lib_dir + 'libclang.dylib'

        if os.path.exists(libclang):
            cindex.Config.set_library_path(os.path.dirname(libclang))

        if os.path.exists(sdk_dir):
            sysroot_dir = os.path.join(sdk_dir, next(os.walk(sdk_dir))[1][0])
            parameters.append('-isysroot')
            parameters.append(sysroot_dir)
    elif platform.system() == 'Linux':
        # clang doesn't find its own base includes by default on Linux,
        # but different distros install them in different paths.
        # Try to autodetect, preferring the highest numbered version.
        def clang_folder_version(d):
            return [int(ver) for ver in re.findall(r'(?<!lib)(?<!\d)\d+', d)]
        clang_include_dir = max((
            path
            for libdir in ['lib64', 'lib', 'lib32']
            for path in glob('/usr/%s/clang/*/include' % libdir)
            if os.path.isdir(path)
        ), default=None, key=clang_folder_version)
        if clang_include_dir:
            parameters.extend(['-isystem', clang_include_dir])
    elif platform.system() == 'Windows':
        parameters.extend(['-fms-compatibility', '-std=c++14'])
    if not any(it.startswith("-std=") for it in parameters):
        parameters.append('-std=c++11')
    for item in args:
        if item.startswith('-'):
            parameters.append(item)
        else:
            filenames.append(item)
    return parameters, filenames

def parse_header(filename: str, additional_include_directories: List[str] = []) -> Scope:
    args = [f"-I{d}" for d in additional_include_directories]
    args.append(filename)
    parameters, filenames = parse_args(args)
    return parse_file(filenames[0], parameters)
    
if __name__ == '__main__':
    openvds_root_dir = os.path.abspath("../..")
    print(f"openvds_root_dir={openvds_root_dir}")
    namespace = "OpenVDS"
    try:
        args = sys.argv[1:]
        args.append(f'-I{openvds_root_dir}/src/OpenVDS/OpenVDS')
        args.append(f'-I{openvds_root_dir}/src/OpenVDS')
        args.append('-fparse-all-comments')
        parameters, filenames = parse_args(args)
        if not filenames:
            filenames = [
                'test.h'
#                f'{openvds_root_dir}/src/OpenVDS/OpenVDS/VolumeData.h',                
#                f'{openvds_root_dir}/src/OpenVDS/OpenVDS/MetadataKey.h',                
            ]
        for p in parameters:
            if p.startswith('-n'):
                namespace = p[2:].strip()
        if filenames:
            for f in filenames:
                scope = parse_file(f, parameters)
                if namespace:
                    scope = scope.find(namespace)
                scope.dump(show_comments=True)
        else:
            raise NoFilenamesError
    except NoFilenamesError:
        print("Header list empty".format(sys.argv[0]))
    except UnknownOptionError as e:
        print("Unknown option '{}'".format(e.args))
