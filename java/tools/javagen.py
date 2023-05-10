
import sys
import io
import os
import traceback
import re
import fnmatch
from typing import Tuple, List, Dict, Set, Union, TextIO

from clang.cindex import Cursor, Type

from more_itertools import peekable

from parse_cpp_header import parse_header, Param, Scope, ScopeDoc

DEBUG_PRINT_SIGNATURES = False # Print the signature of the wrapped functions

_print_exception_call_stacks = False # For debugging
_functioncall_readability_threshold = 2 # Method formatting: if function argument count is greater than this, arguments will be spread across multiple lines

copyright_txt = ''
imports_txt = ''
includes_txt = ''
java_auto_overloads = {}

# Mapping from canonical type to alias, e.g 'Foo::Bar::IntVec<4>' -> 'Foo::Bar::IntVector4'
g_canonical_to_alias: Dict[str, str] = {}

# Mapping from alias to canonical type. There may exist more than one alias for each canonical type.
g_alias_to_canonical: Dict[str, str] = {}

# Worklist of types to generate
g_instantiate_nodes: Dict[str, str] = {}

# Classes that have already been generated
g_generated_classes: Set[str] = []

# The root of the parse tree, the current translation unit.
g_root: Scope = None

def print_callstack(exc):
    if _print_exception_call_stacks:
        traceback.print_exc(file=sys.stderr)

def capfirst(value: str) -> str:
    return value[0].upper() + value[1:]

def remove_empty_lines(text: str) -> str:
    return '\n'.join([l for l in text.splitlines() if l.strip()])

_ignore_types = [
    "OpenVDS::M4", # Should not even be there
    "OpenVDS::VolumeDataPage::Error", # SteinFIXME add this
    "OpenVDS::MetadataKeyRange",
    "OpenVDS::IVolumeDataAccess",
    "OpenVDS::IVolumeDataAccessManager",
    "OpenVDS::IVolumeDataAccessor", 
    "OpenVDS::IVolumeDataReadWriteAccessor",
    "OpenVDS::IVolumeDataReadAccessor",
    "OpenVDS::IVolumeDataReadWriteAccessor",
    "OpenVDS::IHasVolumeDataAccess",
]

_marshaled_value_types = [
]

_enumset_types = [
    "OpenVDS::VolumeDataLayoutDescriptor::Options",
    "OpenVDS::VolumeDataChannelDescriptor::Flags",
]

_prefixes = [ "OpenVDS" ]

_explicit_add_get_prefix = {
#    "IJKCoordinateTransformer$" : ["IJKGrid", "IJKSize", "IJKToVoxelDimensionMap", "IJKToWorldTransform", "WorldToIJKTransform", "IJKAnnotationStart", "IJKAnnotationEnd", "AnnotationsDefined"],
#    "VolumeDataRequest$" : ["RequestID", "BufferByteSize", "BufferDataType"],
#    "VolumeData[2-4]D\w*Accessor\w*$" : ["RegionCount", "Region", "RegionFromIndex", "CurrentRegion"]
}

_cppjava_typemap = {
    "const char *":                                 "String",
    "std::string":                                  "String",
    "OpenVDS::StringWrapper":                       "String",
    "void":                                         "void",
    "bool":                                         "boolean",
    "char":                                         "byte",
    "unsigned char":                                "byte",
    "int8_t":                                       "byte",
    "uint8_t":                                      "byte",
    "short":                                        "short",
    "unsigned short":                               "short",
    "int16_t":                                      "short",
    "uint16_t":                                     "short",
    "int":                                          "int",
    "unsigned int":                                 "int",
    "long":                                         "int",
    "unsigned long":                                "int",
    "int32_t":                                      "int",
    "uint32_t":                                     "int",
    "long long":                                    "long",
    "unsigned long long":                           "long",
    "int64_t":                                      "long",
    "uint64_t":                                     "long",
    "size_t":                                       "long",
    "int":                                          "int",
    "float":                                        "float",
    "double":                                       "double",
}

_cppjni_typemap = {
    "const char *":         "jstring",
    "std::string":          "jstring",
    "OpenVDS::StringWrapper": "jstring",
    "void":                 "void",
    "bool":                 "jboolean",
    "char":                 "jbyte",
    "unsigned char":        "jbyte",
    "int8_t":               "jbyte",
    "uint8_t":              "jbyte",
    "short":                "jshort",
    "unsigned short":       "jshort",
    "int16_t":              "jshort",
    "uint16_t":             "jshort",
    "int":                  "jint",
    "unsigned int":         "jint",
    "long":                 "jint",
    "unsigned long":        "jint",
    "int32_t":              "jint",
    "uint32_t":             "jint",
    "long long":            "jlong",
    "unsigned long long":   "jlong",
    "int64_t":              "jlong",
    "uint64_t":             "jlong",
    "size_t":               "jlong",
    "int":                  "jint",
    "float":                "jfloat",
    "double":               "jdouble",
}

_cppjni_array_typemap = {
    "bool":                 "jbooleanArray",
    "char":                 "jbyteArray",
    "unsigned char":        "jbyteArray",
    "int8_t":               "jbyteArray",
    "uint8_t":              "jbyteArray",
    "short":                "jshortArray",
    "unsigned short":       "jshortArray",
    "int16_t":              "jshortArray",
    "uint16_t":             "jshortArray",
    "int":                  "jintArray",
    "unsigned int":         "jintArray",
    "long":                 "jintArray",
    "unsigned long":        "jintArray",
    "int32_t":              "jintArray",
    "uint32_t":             "jintArray",
    "long long":            "jlongArray",
    "unsigned long long":   "jlongArray",
    "int64_t":              "jlongArray",
    "uint64_t":             "jlongArray",
    "float":                "jfloatArray",
    "double":               "jdoubleArray",
}

_bytebuffer_backed = {
    "OpenVDS::FloatRange":    "FloatRange",
    "OpenVDS::DoubleRange":   "DoubleRange",
    "OpenVDS::IntRange":      "IntRange",
    "OpenVDS::IntVector4":    "IntVector4",
    "OpenVDS::IntVector3":    "IntVector3",
    "OpenVDS::IntVector2":    "IntVector2",
    "OpenVDS::FloatVector4":  "FloatVector4",
    "OpenVDS::FloatVector3":  "FloatVector3",
    "OpenVDS::FloatVector2":  "FloatVector2",
    "OpenVDS::DoubleVector4": "DoubleVector4",
    "OpenVDS::DoubleVector3": "DoubleVector3",
    "OpenVDS::DoubleVector2": "DoubleVector2",
    "OpenVDS::FloatMatrix3x3": "FloatMatrix3x3",
    "OpenVDS::FloatMatrix4x4": "FloatMatrix4x4",
    "OpenVDS::DoubleMatrix3x3": "DoubleMatrix3x3",
    "OpenVDS::DoubleMatrix4x4": "DoubleMatrix4x4",
    "OpenVDS::IJKGridDefinition": "IJKGridDefinition",
    "OpenVDS::VDSIJKGridDefinition": "VDSIJKGridDefinition",
}

_interfaces = {
#    "OpenVDS::MetadataReadAccess",
#    "OpenVDS::MetadataWriteAccess", 
}

_smart_ptr_types = [
    'std::shared_ptr',
    'std::unique_ptr',
]

_vector_types = [
    'std::vector',
    'OpenVDS::VectorWrapper',
    'VectorWrapper',
]

_optional_types = {
    'int':    'Integer',
    'float':  'Float',
    'double': 'Double',
}

class TypeIgnored(RuntimeError):
    pass

def check_ignore_args(args: List[Param]):
    for arg in args:
        t = get_pointee(arg.canonical_type).split('<')[0]
        if t in _ignore_types:
            raise TypeIgnored(f'Ignoring function with parameter of type: {arg.typename}')

def is_marshaled_valuetype(typename: str) -> bool:
    return clean_typename(typename) in _marshaled_value_types

def is_bytebuffer_backed(typename: str, alias: str = '') -> bool:
    typename = resolve_typealias(typename, alias)
    return clean_typename(typename) in _bytebuffer_backed

def is_smartptr_type(typename: str) -> bool:
    return any([typename.startswith(s) for s in _smart_ptr_types])

def is_vector_type(typename: str) -> bool:
    return any([typename.startswith(s) for s in _vector_types])

def is_pass_by_handle(typename: str) -> bool:
    clean_name = clean_typename(typename)
    if clean_name in _cppjava_typemap:
        return False
    if any([clean_name.startswith(p) for p in _prefixes]):
        return True
    elif is_smartptr_type(typename):
        return True
    else:
        return False

def is_array(typename: str) -> bool:
    if '[' in typename and ']' in typename:
        return True
    return False

def get_array_element(typename: str) -> str:
    assert typename.index('[') >= 0
    tmp = typename[:typename.index('[')]
    ignore_tokens = [ 'const', '*', '&', '(', ')' ]
    for t in ignore_tokens:
        tmp = tmp.replace(t, '')
    return tmp.strip()

def get_array_size(typename: str) -> int:
    """returns 0 if size is unspecified"""
    assert typename.index('[') >= 0 and typename.index(']') > 0
    tmp = typename[typename.index('[')+1 : typename.index(']')].strip()
    return int(tmp) if tmp else 0

def is_templated(typename: str) -> bool:
    if '<' in typename and '>' in typename:
        return typename.index('<') > 0 and typename.rindex('>') > typename.index('<')
    else:
        return False

def get_template_name(typename: str) -> str:
    assert is_templated(typename)
    return typename[:typename.index('<')]

def get_template_args(templated_name: str) -> List[str]:
    assert is_templated(templated_name)
    args = templated_name[templated_name.index('<')+1 : templated_name.rindex('>')].split(',')
    args_clean = [a.strip() for a in args]
    return args_clean

def get_template_arg0(templated_name: str) -> str:
    return get_template_args(templated_name)[0] # nasty

def get_pointee(typename: str) -> str:
    if is_smartptr_type(typename):
        return get_template_arg0(typename)
    return clean_typename(typename)

def is_optional_type(typename: str) -> str:
    return 'optional<' in typename

def is_ptr_type(typename: str) -> bool:
    if '*' in typename:
        return True
    elif is_smartptr_type(typename):
        return True
    else:
        return False

def is_ref_type(typename: str) -> bool:
    if '&' in typename:
        return True
    else:
        return False
        
def strip_prefixes(typename: str) -> str:
    for p in _prefixes:
        if typename.startswith(p):
            return typename[len(p) + 2:]
    return typename

def apply_prefixes(stripped_typename: str, prefixed_typename: str) -> str:
    for p in _prefixes:
        if prefixed_typename.startswith(p):
            return p + '::' + stripped_typename
    return stripped_typename

def clean_typename(typename: str) -> str:
    return typename.replace("const", "").replace("*", "").replace("&", "").strip()

def create_type_nice_name(typename: str) -> str:
    if typename[0].islower():
        return typename[0].upper() + typename[1:]
    else:
        return typename

def already_generated(class_name: str):
    global g_generated_classes
    return strip_prefixes(class_name) in g_generated_classes

def register_generated(class_name: str):
    global g_generated_classes
    assert not already_generated(class_name), f'{class_name}'
    g_generated_classes.append(class_name)

def register_typealias(alias: str, canonical_type: str):
    global g_canonical_to_alias
    global g_alias_to_canonical
    global g_instantiate_nodes
    assert 'long long' not in canonical_type
    assert 'type-parameter-' not in canonical_type
    if not canonical_type in g_canonical_to_alias:
        g_canonical_to_alias[canonical_type] = alias
    if alias in g_alias_to_canonical:
        assert g_alias_to_canonical[alias] == canonical_type
    else:
        g_alias_to_canonical[alias] = canonical_type
    if not already_generated(alias) and alias not in g_instantiate_nodes and not inhibit_generation(alias):
        g_instantiate_nodes[alias] = canonical_type

def lookup_canonical_type(canonical_type: str) -> str:
    if canonical_type in g_canonical_to_alias:
        return g_canonical_to_alias[canonical_type]
    else:
        return canonical_type

def lookup_typealias(alias: str) -> str:
    if alias in g_alias_to_canonical:
        return g_alias_to_canonical[alias]
    else:
        return alias

def register_instantiate(node: Type, instantiate_args: List[str]) -> str:
    assert isinstance(node, Type)
    global g_instantiate_nodes
    global g_generated_classes
    alias = node.spelling
    name = node.get_canonical().spelling
    canonical = Scope.get_node_fullname(node, strip_prefixes(name))
    return register_instantiate_with_parameters(canonical, instantiate_args, alias)

def register_instantiate_with_parameters(canonical: str, instantiate_args: List[str], alias: str = None) -> str:
    assert '<' in canonical
    assert isinstance(canonical, str)
    assert is_templated(canonical)
    if instantiate_args:
        canonical_template_args = get_template_args(canonical)
        used_template_args = []
        if 'type-parameter-' in canonical:
            used_template_args = typename_get_used_template_args(canonical, instantiate_args)
        elif alias and '<' in alias:
            used_template_args = get_template_args(alias)
        if used_template_args:
            args_dict = get_template_args_dict(instantiate_args, used_template_args)
            substituted_template_args = substitute_template_args(canonical_template_args, args_dict)
            canonical = typename_substitute_template_args(canonical, substituted_template_args)
            alias = make_template_instantiated_name(get_template_name(canonical), substituted_template_args)
        else:
            raise NotImplementedError("Failed to deduce template parameters")
        if canonical in g_canonical_to_alias:
            alias = g_canonical_to_alias[canonical]
        else:
            alias = make_template_instantiated_name(get_template_name(canonical), substituted_template_args)
            register_typealias(alias, canonical)
    if not alias:
        raise NotImplementedError("Failed to deduce template parameters")
    return alias

def param_to_real_type(param: Param, template_args: List[str]) -> str:
    if is_templated(param.canonical_type):
        if not is_smartptr_type(param.canonical_type):
            if not 'std' in param.canonical_type: # eeh..
                  return register_instantiate(param.typenode, template_args)
            return param.typename
    if is_smartptr_type(param.canonical_type):
        return param.typename
    return param.canonical_type

def resolve_typealias(typename: str, alias: str) -> str:
    if alias and '<' in typename:
        if '<' in alias:
            return typename # To avoid losing namespace of template parameter...
        typename = alias
    return typename

def _cpp_to_java_type(typename: str, alias: str) -> str:
    typename = resolve_typealias(typename, alias)
    if find_enum_type(typename):
        if typename in _enumset_types:
            return "EnumSet<" + strip_prefixes(clean_typename(typename)).replace('::', '.') + ">"
        if typename in _cppjava_typemap:
            return _cppjava_typemap[typename]
        else:
            return strip_prefixes(clean_typename(typename)).replace('::', '.')
    elif is_marshaled_valuetype(typename):
        t = clean_typename(typename)
        if t in _cppjava_typemap:
            return _cppjava_typemap[t]
        else:
            return strip_prefixes(clean_typename(typename))
    elif is_array(typename):
        element_type = get_array_element(typename)
        if not element_type in _cppjni_array_typemap:
            raise NotImplementedError(f"Unsupported array type: {element_type}")
        java_element_type = cpp_to_java_type(element_type)
        return f"{java_element_type}[]"
    elif is_vector_type(typename):
        element_type = get_template_arg0(typename)
        if element_type in _cppjni_array_typemap or is_pass_by_handle(element_type):
            java_element_type = cpp_to_java_type(element_type)
            return f"{java_element_type}[]"
        else:
            raise NotImplementedError(f"Unsupported vector type: {element_type}")
    elif is_bytebuffer_backed(typename):
        return _bytebuffer_backed[clean_typename(typename)]
    elif is_smartptr_type(typename):
        return get_pointee(typename)
    elif is_pass_by_handle(typename):
        return strip_prefixes(clean_typename(typename))
    elif typename in _cppjava_typemap:
        return _cppjava_typemap[typename]
    elif is_optional_type(typename):
        assert False, "We should never get here"
    clean_type = typename if not 'const' in typename else typename.replace('&', '').replace('const', '').strip()
    clean_type = clean_type if not '&&' in clean_type else clean_type.replace('&&', '').strip()
    if typename in _cppjava_typemap:
        return _cppjava_typemap[typename]
    elif clean_type in _cppjava_typemap:
        return _cppjava_typemap[clean_type]
    else:
        raise NotImplementedError(f'Unhandled type: {typename}')

def cpp_to_java_type(typename: str, alias: str = '') -> str:
    j = _cpp_to_java_type(typename, alias)
    if '::' in j:
        return strip_prefixes(clean_typename(j)).replace('::', '.')
    else:
        return j

def cpp_to_native_type(typename: str, alias: str = '') -> str:
    typename = resolve_typealias(typename, alias)
    assert typename != 'jlong'
    if find_enum_type(typename):
        return "long"
    elif is_marshaled_valuetype(typename):
        return cpp_to_java_type(typename)
    elif is_array(typename):
        element_type = get_array_element(typename)
        if not element_type in _cppjni_array_typemap:
            raise NotImplementedError(f"Unsupported array type: {element_type}")
        java_element_type = cpp_to_native_type(element_type)
        return f"{java_element_type}[]"
    elif is_vector_type(typename):
        element_type = get_template_arg0(typename)
        if element_type in _cppjni_array_typemap:
            java_element_type = cpp_to_native_type(element_type)
        elif is_pass_by_handle(element_type):
            java_element_type = 'long'
        else:
            raise NotImplementedError(f"Unsupported vector type: {element_type}")
        return f"{java_element_type}[]"
    elif is_bytebuffer_backed(typename):
        return "ByteBuffer"
    elif is_optional_type(typename):
        assert False, "We should never get here"
    elif is_pass_by_handle(typename):
        return "long"
    else:
        return cpp_to_java_type(typename)
        
def cpp_to_jni_type(typename: str, alias: str = '') -> str:
    assert typename != 'jlong'
    typename = resolve_typealias(typename, alias)
    if is_array(typename):
        element_type = get_array_element(typename)
        if not element_type in _cppjni_array_typemap:
            raise NotImplementedError(f"Unsupported array type: {element_type}")
        return _cppjni_array_typemap[element_type]
    elif is_marshaled_valuetype(typename):
        t = clean_typename(typename)
        if t in _cppjni_typemap:
            return _cppjni_typemap[t]
        else:
            return 'jobject'
    elif is_vector_type(typename):
        element_type = get_template_arg0(typename)
        if element_type in _cppjni_array_typemap:
            return _cppjni_array_typemap[element_type]
        elif is_pass_by_handle(element_type):
            return _cppjni_array_typemap['int64_t']
        else:
            raise NotImplementedError(f"Unsupported vector type: {element_type}")
    elif is_bytebuffer_backed(typename):
        assert False, "We should never get here"
    elif is_optional_type(typename):
        assert False, "We should never get here"
    elif find_enum_type(typename):
        return "jlong"
    elif is_pass_by_handle(typename):
        return "jlong"
    elif typename in _cppjni_typemap:
        return _cppjni_typemap[typename]
    else:
        clean_type = typename if not 'const' in typename else typename.replace('&', '').replace('const', '').strip()
        clean_type = clean_type if not '&&' in clean_type else clean_type.replace('&&', '').strip()
        if clean_type in _cppjni_typemap:
            return _cppjni_typemap[clean_type]
        else:
            raise NotImplementedError(f'Unhandled type: {typename}')

def create_jni_arglist(class_name: str, args: List[Param], is_include_proxyinterface_arg: bool=False, is_static_method: bool=False) -> str:
    arglist = [ 'JNIEnv * env' ]
    if is_static_method:
        arglist.append('jclass cls')
    else:
        arglist.append('jobject object')
    if is_static_method:
        pass
    else:
        arglist.append('jlong native_handle')
    p = peekable(args)
    for arg in p:
        type_, alias_, name = arg.canonical_type, arg.typename, arg.name
        if is_bytebuffer_backed(type_, alias_):
            # bytebuffer objects are passed as 2 separate parameters: the bytebuffer object itself and a byteoffset into the buffer
            arglist.append(f"jobject {name}bytebuffer, jlong {name}byteoffset")
        elif is_optional_type(type_):
            # optional<T> values are passed as 2 separate parameters: the T value and whether the T value is valid.
            t = get_template_arg0(type_)
            jnitype_ = cpp_to_jni_type(t)
            arglist.append(f"{jnitype_} {name}, jboolean use_{name}")
        elif is_array(type_):
            jnitype_ = cpp_to_jni_type(type_)
            arglist.append(f"{jnitype_} {name}")
            if '*' in type_ and is_arg_size_type(p.peek(None)):
                next(p) # Skip array size. It is obtained from the array object.
        elif use_buffer_protocol([arg, p.peek(None)]):
            arglist.append(f"jobject {name}")
            next(p) # consume buffer size parameter
        else:
            jnitype_ = cpp_to_jni_type(type_, alias_)
            arglist.append(jnitype_ + " " + name)
    return ", ".join(arglist)        

def transform_jni_functioncall_args(args: List[Param], is_static_method: bool=False) -> Tuple[str, str, str]:
    arglist = []
    prologue = []
    epilogue = []
    p = peekable(args)
    if args and p.peek().is_out:
        result: Param = next(p)
        typename = clean_typename(result.typename)
        name = result.name
        assert is_bytebuffer_backed(result.canonical_type, result.typename) # This is the only way we should get here.
        epilogue.append(f'*({typename}*)((char*)env->GetDirectBufferAddress({name}bytebuffer) + {name}byteoffset) = result;')
    for arg in p:
        name = arg.name
        type_ = resolve_typealias(arg.canonical_type, arg.typename)
        if is_array(type_):
            arr_element_type = get_array_element(type_)
            arr_size = get_array_size(type_)
            arr_addr = '&' if '*' in type_ else ''
            is_mutable = 'false'
            if not 'const' in type_:
                is_mutable = 'true'
            prologue.append(f"auto tmp{name} = CPPJNIArrayAdapter<{arr_element_type},{arr_size},{is_mutable}>(env, {name});")
            if '*' in type_ and is_arg_size_type(p.peek(None)):
                nextarg = next(p)
                ntype_, nname = nextarg.canonical_type, nextarg.name
                arglist.append(f"{arr_addr}tmp{name}.getArray(), ({ntype_})tmp{name}.getArrayLength()")
            else:
                arglist.append(f"{arr_addr}tmp{name}.getArray()")
        elif is_marshaled_valuetype(type_):
            t = type_ if '*' in type_ else clean_typename(type_)
            initializer = 'nullptr' if '*' in t else f'{t}()'
            prologue.append(f'{t} tmp{name} = {initializer};')
            prologue.append(f'Marshaling::Convert(tmp{name}, {name});')
            arglist.append(f'tmp{name}')
        elif is_bytebuffer_backed(type_):
            # bytebuffer objects are passed as 2 separate parameters: the bytebuffer object itself and a byteoffset into the buffer
            cleantype = clean_typename(type_)
            arglist.append(f"CPPJNIByteBufferAdapter<{cleantype}>(env, {name}bytebuffer, {name}byteoffset)")
        elif is_optional_type(type_):
            # optional<T> values are passed as 2 separate parameters: the T value and whether the T value is valid.
            t = get_template_arg0(type_)
            arglist.append(f"use_{name} ? OpenVDS::optional<{t}>({name}) : OpenVDS::optional<{t}>()")
        elif use_buffer_protocol([arg, p.peek(None)]):
            # get size parameter from bytebuffer object
            elem_type = get_pointee(type_)
            prologue.append(f"auto tmp{name} = CPPJNIAsyncBuffer<{elem_type}>(env, {name});")
            arglist.append(f"tmp{name}.buffer(), tmp{name}.byteSize()")
            if 'const' in type_:
                # Buffer is read-only, so its lifetime is not assumed to be longer than the duration of this call,
                # so no need to keep it alive by adding a global reference.
                pass
            else:
                # Keep it alive!
                epilogue.append(f"context->registerGlobalRef(env, {name});")
            next(p) # consume buffer size parameter
        elif  is_vector_type(type_) and is_pass_by_handle(clean_typename(get_template_arg0(type_))):
            elem_type = get_template_arg0(type_)
            arglist.append(f'CPPJNIVectorWrapperAdapter<{elem_type}>(env, {name}).toVector()')
        else:
            jnitype_ = cpp_to_jni_type(type_)
            if jnitype_ == "jstring":
                if is_static_method:
                    _wrapper = f"CPPJNIStringWrapper(env, {name})"
                else:
                    _wrapper = f"CPPJNIStringWrapper(env, native_handle, {name})"
                _clean_type = clean_typename(type_)
                if not 'const char *' in type_:
                    # we want to avoid overload ambiguities with regard to const char*/std::string etc
                    _wrapper = f'{_clean_type}({_wrapper})'
                arglist.append(_wrapper)
            elif jnitype_ == "jboolean":
                arglist.append(f"{name} ? true : false")
            elif is_enum_type(arg):
                # We want to avoid any clashes between function names and type names,
                # so lets's be specific here:
                if is_scoped_enum_type(arg) and not is_enum_class(arg):
                    arglist.append(f"(enum {type_}){name}")
                else:
                    arglist.append(f"({type_}){name}")
            elif is_pass_by_handle(type_):
                cpptype = clean_typename(type_)
                if '&' in type_:
                    arglist.append(f"*CPPJNI_cast<{cpptype}>({name})")
                else:
                    arglist.append(f"CPPJNI_cast<{cpptype}>({name})")
            else:
                arglist.append(name)
    separator = '\n                               ' if len(args) > _functioncall_readability_threshold else ''
    _args = ", ".join([separator + a for a in arglist])
    _prologue = "\n".join(['    ' + p for p in prologue])
    _epilogue = "\n".join(['    ' + e for e in epilogue])
    return _args, _prologue, _epilogue

def create_jni_proto(retval: Param, class_name: str, method_name: str, extra_args: List[Param]=[], is_include_proxyinterface_arg: bool=False, is_static_method: bool=False) -> str:
    assert isinstance(retval, Param)
    return_type = cpp_to_jni_type(retval.canonical_type)
    args = create_jni_arglist(class_name, extra_args, is_include_proxyinterface_arg, is_static_method)
    proto = "\nJNIEXPORT {} JNICALL Java_org_opengroup_openvds_{}_{}Impl\n  ({})".format(return_type, class_name, method_name, args)
    return proto

def _override_fullname(scope: Scope, class_name: str) -> str:
    if scope.is_namespace:
        fullname = f'{scope.fullname}::{class_name}'
    else:
        fullname = '::'.join([scope.prefix, class_name])
    return fullname

def create_jni_ctor(scope: Scope, ctor: Scope, class_name: str, class_canonical_name: str, overload_name: str, overloads_created: List[str], extra_args: List[Param]=[]) -> str:
    class_fullname = _override_fullname(scope, class_name)
    overload_name = overload_name.replace(ctor.name, "ctor")
    proto = create_jni_proto(Param(class_fullname), class_name, overload_name, extra_args=extra_args, is_include_proxyinterface_arg=True, is_static_method=True)
    if proto in overloads_created:
        return ''
    args, prologue, epilogue = transform_jni_functioncall_args(extra_args, is_static_method=True)
    body = f"""
{{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {{
    auto context = CPPJNI_createObjectContext<{class_canonical_name}>();
{prologue}
    auto native_handle = context->handle();
    context->setObject(CPPJNI_makeShared<{class_canonical_name}>({args}));
{epilogue}
    return native_handle;
  }}
  CPPJNI_CATCH
  return 0;
}}
"""
    overloads_created.append(proto)
    return proto + body

def create_jni_dtor(scope: Scope, class_name: str, class_canonical_name: str) -> str:
    proto = create_jni_proto(Param('void'), class_name, "dtor", is_include_proxyinterface_arg=False, extra_args=[Param('bool', 'is_disposing')])
    body = f"""
{{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {{
    CPPJNI_destroyHandle<{class_canonical_name}>(native_handle, is_disposing);
  }}
  CPPJNI_CATCH
}}
"""
    return proto + body

def create_jni_equals(class_name, class_canonical_name):
    proto = create_jni_proto(Param('bool'), class_name, "operatorEQ", extra_args=[Param(typename=class_canonical_name, name='other_native_handle')], is_include_proxyinterface_arg=False, is_static_method=False)
    body = f"""
{{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {{
    auto pInstance = CPPJNI_cast<{class_canonical_name}>(native_handle);
    auto pOtherInstance = CPPJNI_cast<{class_canonical_name}>(other_native_handle);
    return *pInstance == *pOtherInstance;
  }}
  CPPJNI_CATCH
  return 0;
}}
"""
    return proto + body

def find_enum_type(real_return_type: str) -> Scope:
    result = g_root.try_find(real_return_type)
    return result if result and result.is_enum else None

def is_string_type(arg: Param) -> bool:
    assert isinstance(arg, Param)
    return cpp_to_java_type(arg.canonical_type, arg.typename) == "String"

def is_enum_type(arg: Param) -> bool:
    assert isinstance(arg, Param)
    return True if find_enum_type(arg.canonical_type) else False

def is_enumset_type(arg: Param) -> bool:
    assert isinstance(arg, Param)
    if is_enum_type(arg):
        if arg.canonical_type in _enumset_types:
            return True
    return False

def is_enum_class(arg: Param) -> bool:
    t = find_enum_type(arg.canonical_type)
    if t:
        return t.is_enum_class
    return False

def is_scoped_enum_type(arg: Param) -> bool:
    t = find_enum_type(arg.canonical_type)
    if t:
        if t.parent.is_record:
            return True
    return False

def create_jni_property_setter(scope: Scope, class_name: str, overload_name: str, property_def: Param, class_canonical_name: str) -> str:
    value = Param(name='value', typename=property_def.typename, canonical_type=property_def.canonical_type)
    args = [ value ]
    arglist = create_jni_arglist(class_name, args)
    proto = create_jni_proto(Param('void'), class_name, overload_name, is_include_proxyinterface_arg=True, extra_args=args)
    invoke_args, prologue, epilogue = transform_jni_functioncall_args(args)
    assert not prologue
    assert not epilogue
    body = f"""
{{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {{
    auto pInstance = CPPJNI_cast<{class_canonical_name}>(native_handle);
    pInstance->{scope.name} = {invoke_args};
  }}
  CPPJNI_CATCH
}}
"""
    return proto + body

def create_jni_methods(scope: Scope, template: str, override_name: str = '', template_args: List[str] = [], explicit_children: List[Scope] = []):
    output = io.StringIO()
    class_name = override_name or scope.name
    classname_full = _override_fullname(scope, class_name)
    if template_args:
        assert not '<' in scope.fullname
        class_canonical_name = f'{scope.fullname}<' + ', '.join(template_args) + '>'
    else:
        class_canonical_name = classname_full
    has_ctor = False
    has_instance_methods = False
    ignored_nodes = get_ignored_nodes_from_template(template)
    failed_nodes = []
    overloads_created = []
    bases = scope.get_base_classes()
    scopes = [ scope ]
    if len(bases) > 1:
        # Multiple inheritances is only indirectly supported. Members of base classes other than the first
        # are inlined like normal class methods.
        if not scope.is_namespace:
            scopes.extend([b for b,t in bases[1:]])
            bases = [bases[0]]
    for _scope in scopes:
        children = explicit_children or _scope.get_children()
        for child in children:
            if str(child) in ignored_nodes:
                continue
            local_output = io.StringIO()
            try:
                if child.is_function or child.is_data_member:
                    if child.is_template:
                        continue
                    if child.is_destructor:
                        continue
                    result_param = child.result or Param(child.typename, 'result', is_out=True)                    
                    args = child.get_args()
                    if template_args:
                        result_param = param_subsitute_template_args(result_param, template_args)
                        args = arglist_substitute_template_args(args, template_args)
                    instantiated_result_param = result_param
                    if is_bytebuffer_backed(result_param.canonical_type, result_param.typename):
                        args.insert(0, result_param)
                        result_param = Param(typename='void', is_out=True)
                        real_return_type =  resolve_typealias(result_param.canonical_type, result_param.typename)
                    check_ignore_args(args)
                    check_ignore_args([result_param])
                    real_return_type = result_param.canonical_type
                    return_type = cpp_to_jni_type(result_param.canonical_type)
                    method_name = child.name
                    overload_name = child.overload_name
                    if child.is_data_member:
                        method_name = f'get{capfirst(child.name)}'
                        overload_name = method_name
                    if method_name.startswith("operator"):
                        if method_name == "operator==":
                            method = create_jni_equals(class_name, class_canonical_name)
                            if DEBUG_PRINT_SIGNATURES:
                                print(f'\n///AUTOGEN-OK: {child}', file=local_output, end='')
                            print(method, file=local_output)
                            has_instance_methods = True
                    elif child.is_constructor:
                        if is_ctor_valid(child):
                            if DEBUG_PRINT_SIGNATURES:
                                print(f'\n///AUTOGEN-OK: {child}', file=local_output, end='')
                            print(create_jni_ctor(scope, child, class_name, class_canonical_name, overload_name, overloads_created, args), file=local_output)
                            has_instance_methods = True
                            has_ctor = True
                    else:
                        is_static_method = child.is_static_method or scope.is_namespace
                        proto = create_jni_proto(result_param, class_name, overload_name, extra_args=args, is_include_proxyinterface_arg=True, is_static_method=is_static_method)
                        overload_signature = proto
                        if overload_signature in overloads_created:
                            continue
                        overloads_created.append(overload_signature)
                        if DEBUG_PRINT_SIGNATURES:
                            print(f'\n///AUTOGEN-OK: {child}', file=local_output, end='')
                        print(proto, file=local_output)
                        declare_result = 'auto result = ' if child.is_data_member or not child.result.canonical_type == 'void' else ''
                        if is_ref_type(real_return_type):
                            if is_pass_by_handle(real_return_type) or is_vector_type(real_return_type):
                                declare_result = 'auto& result = '
                        retval = ''
                        print("{", file=local_output)
                        print("  JNIEnvGuard\n    envGuard(env);\n", file=local_output)
                        print("  CPPJNI_TRY\n  {", file=local_output)
                        invoke_args, prologue, epilogue = transform_jni_functioncall_args(args, is_static_method=is_static_method)
                        if prologue:
                            print(prologue, file=local_output)
                        if is_static_method:
                            print("    {}{}({});".format(declare_result, child.fullname, invoke_args), file=local_output)
                        else:
                            print("    auto pInstance = CPPJNI_cast<{}>(native_handle);".format(class_canonical_name), file=local_output)
                            if child.is_data_member:
                                print("    {}pInstance->{};".format(declare_result, child.name), file=local_output)
                            else:
                                print("    {}pInstance->{}({});".format(declare_result, child.name, invoke_args), file=local_output)
                        if return_type == 'jstring':
                            retval = 'CPPJNI_newString(env, result)'
                        elif is_marshaled_valuetype(real_return_type):
                            marshaled_type = cpp_to_jni_type(real_return_type)
                            print(f'    {marshaled_type} real_result = {marshaled_type}();', file=local_output)
                            print(f'    Marshaling::Convert(real_result, result);', file=local_output)
                            retval = 'real_result'
                        elif is_vector_type(real_return_type):
                            element_type = get_template_arg0(real_return_type)
                            if is_pass_by_handle(element_type):
                                raise NotImplementedError(f'Return values of type {element_type}as array.')
                            else:
                                retval = f'CPPJNIVectorAdapter<{element_type}>(env, result).toArray()'
                        elif is_pass_by_handle(real_return_type):
                            if is_enum_type(result_param):
                                retval = '(jlong)result'
                            elif is_smartptr_type(real_return_type):
                                retval = 'CPPJNI_createObjectContext(result)'.format(real_return_type)
                            elif is_ptr_type(real_return_type) or is_ref_type(real_return_type):
                                creator = '' if is_static_method else ', native_handle, pInstance'
                                if is_ptr_type(real_return_type):
                                    # This handle will not destroy the backing object because it is not the owner:
                                    retval = f'CPPJNI_createNonOwningObjectContext(result{creator})'
                                else:
                                    # This handle will not destroy the backing object because it is not the owner:
                                    retval = f'CPPJNI_createNonOwningObjectContext(&result{creator})'
                            else:
                                retval = 'CPPJNI_createObjectContext(CPPJNI_makeShared<{}>(result))'.format(clean_typename(real_return_type))
                        elif return_type == 'void':
                            pass
                        else:
                            retval = 'result'
                        if retval:
                            if 'ObjectContext' in retval:
                                print(f'    auto context = {retval};', file=local_output)
                        if epilogue:
                            print(epilogue, file=local_output)
                        if retval:
                            if 'ObjectContext' in retval:
                                print('    return context->handle();', file=local_output)
                            else:
                                print(f'    return {retval};', file=local_output)
                        if 'context->' in epilogue and not 'ObjectContext' in retval:
                            raise NotImplementedError(f'Protocol mismatch: No local context.')
                        print("  }\n  CPPJNI_CATCH", file=local_output)
                        if not return_type == "void":
                            print("  return 0;", file=local_output)
                        print("}", file=local_output)
                        if child.is_data_member:
                            property_setter = create_jni_property_setter(child, class_name, f'set{capfirst(child.name)}', instantiated_result_param, class_canonical_name)
                            if property_setter:
                                print(property_setter, file=local_output)
                        if not is_static_method:
                            has_instance_methods = True
            except NotImplementedError as e:
                print(f'///AUTOGEN-FAIL: {child}', file=output)
                failed_nodes.append(child)
                print("\nC++: While parsing {}, the following exception occurred:".format(child))
                print(e, file=sys.stderr)
                print_callstack(e)
            except TypeIgnored as e:
#                print(f'\nWhile parsing {child}, the following exception occurred:', file=sys.stderr)
#                print(e, file=sys.stderr)
#                print_callstack(e)
                pass
            else:
                output.write(local_output.getvalue())
    if has_instance_methods:
        if not '_dtorImpl' in template:
            dtor = create_jni_dtor(scope, class_name, class_canonical_name)
            print(dtor, file=output)
    if failed_nodes:
        print(f'C++: Failed nodes for {class_name}:', file=sys.stderr)
        for n in failed_nodes:
            print(f'///AUTOGEN-FAIL: {n}', file=sys.stderr)
    return output.getvalue() + template # no substitutions in template, just plain old code

buffer_types = [ 
    'void', 
    'unsigned char',
    'uint8_t', 
    'uint16_t', 
    'unsigned short',
    'uint32_t', 
    'unsigned long',
    'unsigned int',
    'unsigned long long'
    'uint64_t', 
    'int8_t', 
    'char',
    'int16_t', 
    'short',
    'int32_t', 
    'int',
    'long'
    'int64_t', 
    'long long',
    'float',
    'double'  
]

size_types = [
  'int',
  'size_t',
  'int64_t',
  'long long',
]

def is_arg_buffer_ptr(arg: Param) -> bool:
    t = arg.typename
    if '*' in t and any([bt in t for bt in buffer_types]):
        return True
    else:
        return False

def is_arg_size_type(arg: Param) -> bool:
    t = arg.typename
    if is_ptr_type(t):
        return False
    if any([st in t for st in size_types]):
        return True
    return False

def use_buffer_protocol(args: List[Param]) -> bool:
    if len(args) < 2:
        return False
    if 'const' in args[0].canonical_type and clean_typename(args[0].canonical_type) != 'void':
        return False
    return is_arg_buffer_ptr(args[0]) and args[1] and is_arg_size_type(args[1])
        
def create_native_arglist(class_name:str, args: List[Param], is_include_proxyinterface_arg: bool=False, is_static_method: bool=False) -> str:
    arglist = []
    if not is_static_method:
        arglist.append('long native_object')
    p = peekable(args)
    for arg in p:
        type_, alias_, name_ = arg.canonical_type, arg.typename, arg.name
        if use_buffer_protocol([arg, p.peek(None)]):
            arglist.append(f'ByteBuffer {name_}')
            next(p)
        elif is_optional_type(type_):
            otype = get_template_arg0(type_)
            arglist.append(f'{otype} {name_}, boolean use_{name_}')
        elif is_bytebuffer_backed(type_, alias_):
            arglist.append(f'ByteBuffer {name_}, long {name_}_byteoffset')
        else:
            nativetype_ = cpp_to_native_type(type_, alias_)
            arglist.append(nativetype_ + " " + name_)
    return ", ".join(arglist)        

def create_java_arglist(args: List[Param]) -> str:
    arglist = []
    p = peekable(args)
    for arg in p:
        type_, alias_, name_ = arg.canonical_type, arg.typename, arg.name
        if use_buffer_protocol([arg, p.peek(None)]):
            arglist.append(f'ByteBuffer {name_}')
            next(p)
        elif is_optional_type(type_):
            otype = get_template_arg0(type_)
            if otype not in _optional_types:
                raise NotImplementedError(f'Unsupported optional type: {otype}')
            objtype = _optional_types[otype]
            arglist.append(f'{objtype} {name_}')
        else:
            nativetype_ = cpp_to_java_type(type_, alias_)
            arglist.append(f'{nativetype_} {name_}')
    return ", ".join(arglist)        

def add_parameter_null_check(type_: str, name_: str) -> str:
    return f'ManagedBase.requireNonNull({name_}, "{name_} may not be null")'

def transform_java_functioncall_args(class_name: str, args: List[Param], is_include_proxyinterface_arg: bool=False, is_static_method: bool=False, used_arglist = []) -> Tuple[str, str, str]:
    arglist = []
    if not is_static_method:
        arglist.append('getNativeObject()')
    prologue = []
    epilogue = []
    p = peekable(args)

    # Make sure parameter for any explicitly sized array arg is the right length
    for arg in p:
        type_, name_ = arg.canonical_type, arg.name
        if is_array(type_):
            prologue.append(add_parameter_null_check(type_, name_) + ';')
            array_size = get_array_size(type_)
            if array_size > 0:
                prologue.append(f'if ({name_}.length != {array_size}) throw new IllegalArgumentException("Array \\"{name_}\\" must have length {array_size}");')
    p = peekable(args)
    if args and p.peek().is_out:
        result: Param = next(p)
        typename = cpp_to_java_type(result.typename, result.typename)  #clean_typename(result.typename)
        name_ = result.name
        assert is_bytebuffer_backed(result.canonical_type, result.typename) # This is the only way we should get here.
        prologue.append(f'{typename} {name_} = new {typename}();')
        arglist.append(f'{name_}.getBackingByteBuffer(), {name_}.getByteBufferOffset()')
        epilogue.append(f'return {name_};')
    for arg in p:
        type_, alias_, name_ = arg.canonical_type, arg.typename, arg.name
        checked_name_ = add_parameter_null_check(type_, name_)
        type_ = resolve_typealias(type_, alias_)
        used_arglist.append(name_)
        if is_enumset_type(arg):
            _javatype = cpp_to_java_type(arg.canonical_type).replace('EnumSet<', '').replace('>', '')
            arglist.append(f'{_javatype}.valueFromSet({checked_name_})')
        elif is_enum_type(arg):
            arglist.append(f'{checked_name_}.value()')
        elif is_vector_type(type_) and is_pass_by_handle(clean_typename(get_template_arg0(type_))):
            prologue.append(f'long[] {name_}tmp = new long[{checked_name_}.length];')
            prologue.append(f'for (int i = 0; i < {name_}.length; ++i) {{')
            prologue.append(f'    {name_}tmp[i] = {name_}[i].getNativeObject();')
            prologue.append(f'}}')
            arglist.append(f'{name_}tmp')
        elif is_marshaled_valuetype(type_):
            arglist.append(checked_name_)
        elif is_bytebuffer_backed(type_):
            arglist.append(f'{checked_name_}.getBackingByteBuffer(), {checked_name_}.getByteBufferOffset()')
        elif use_buffer_protocol([arg, p.peek(None)]):
            arglist.append(f'ManagedBuffer.ensureByteBufferValid({name_})')
            next(p)
        elif is_optional_type(type_):
            otype = get_template_arg0(type_)
            arglist.append(f'{name_} == null ? ({otype})0 : ({otype}){name_}, {name_} != null')
        elif is_pass_by_handle(type_):
            arglist.append(f"{checked_name_}.getNativeObject()")
        elif is_string_type(arg):
            arglist.append(checked_name_)
        else:
            arglist.append(name_)
    _args = ", ".join(arglist)
    _prologue = "\n".join(['        ' + p for p in prologue])
    _epilogue = "\n".join(['        ' + e for e in epilogue])
    return _args, _prologue, _epilogue

def create_java_property_setter(scope: Scope, class_name: str, overload_name: str, property_def: Param) -> str:
    value = Param(name='value', typename=property_def.typename, canonical_type=property_def.canonical_type)
    args = [ value ]
    native_arglist = create_native_arglist(class_name, args)
    native_arglist = create_native_arglist(class_name, args, True)
    arglist = create_java_arglist(args)
    overload_signature = f'{overload_name}({arglist})'
    fcall, prologue, epilogue = transform_java_functioncall_args(class_name, args, True)
    docstring = format_docstring(scope, indent='    ', used_arglist=args)
    function_docstring = f'\n{docstring}' if docstring else ''
    ctor = f"""
    native private void {overload_name}Impl({native_arglist});
    {function_docstring}
    public void {overload_name}({arglist}) {{
    {prologue}
        {overload_name}Impl({fcall});    
    {epilogue}
    }}"""
    return ctor

def create_java_ctor(scope: Scope, class_name: str, overload_name: str, has_baseclass: bool, overloads_created: List[str], extra_args: List[Param] = []) -> str:
    overload_name = overload_name.replace(scope.name, "ctor")
    native_arglist = create_native_arglist(class_name, extra_args, True, True)
    arglist = create_java_arglist(extra_args)
    overload_signature = f'{class_name}({arglist})'
    if overload_signature in overloads_created:
        return ''
    overloads_created.append(overload_signature)
    used_args = [] 
    fcall, prologue, epilogue = transform_java_functioncall_args(class_name, extra_args, True, True, used_arglist=used_args)
    docstring = format_docstring(scope, indent='    ', used_arglist=used_args)
    function_docstring = f'\n{docstring}' if docstring else ''
    if has_baseclass:
        ctor = f"""
    native private static long {overload_name}Impl({native_arglist});
    {function_docstring}
    public {class_name}({arglist}) {{
    {prologue}
        super({overload_name}Impl({fcall}));
    {epilogue}
    }}"""
    else:
        ctor = f"""
    native private static long {overload_name}Impl({native_arglist});
    {function_docstring}
    public {class_name}({arglist}) {{
    {prologue}
        this.native_object = {overload_name}Impl({fcall});    
    {epilogue}
    }}"""
    return ctor

def create_java_equals(scope):
    class_name = scope.name
    method = f"""
    native private boolean operatorEQImpl(long nativeobject, long othernativeobject);
    
    @Override
    public boolean equals(Object o) {{
        if (this == o)
            return true;
        if (o == null)
            return false;
        if (getClass() != o.getClass())
            return false;
        {class_name} other = ({class_name})o;
        return operatorEQImpl(getNativeObject(), other.getNativeObject());
    }}"""
    return method

def indent(txt: str, line_prefix = '    '):
    return "\n".join([ line_prefix + l for l in txt.splitlines()])

def _apply_template(template__, body__, **kwargs):
    defaults = {
        'class_visibility': 'public ',
        'class_type': 'class',
        'class_name': 'Foo',
        'class_extends_txt': '',
        'class_implements_txt': '',
        'class_docstring': '',
        'class_body': body__,
    }
    args = { **defaults, **kwargs }
    return template__.format(**args)

def _is_all_caps_enum_value(enum_value_name):
    return re.match('[A-Z_]*$', enum_value_name) != None
    
def _is_any_not_all_caps_enum_value(enum_values):
    for n,v,d in enum_values:
        if not _is_all_caps_enum_value(n):
            return True
    return False

_java_enum_integral_types = {
                  # (javatype, jnitype, literal-suffix)
    'int':          ('int',  'jint',  '' ),
    'unsigned int': ('int',  'jint',  ''),
    'int64_t':      ('long', 'jlong', 'L' ),
    'uint64_t':     ('long', 'jlong', 'L' ),
}

def create_java_enum(scope: Scope, template):
    def fix_enum_name(n: str) -> str:
#        if n.startswith(enum_name):
#            fixed = n.replace(enum_name + '_', '')
#            if fixed[0].isnumeric():
#                return '_' + fixed
#            else:
#                return fixed
        return n

    assert scope.is_enum
    enum_name = scope.name
    if scope.enum_integral_type not in _java_enum_integral_types:
        raise NotImplementedError(f'enum integral type {scope.enum_integral_type} not supported')
    int_type, jni_type, literal_suffix = _java_enum_integral_types[scope.enum_integral_type]
    is_enumset = scope.fullname in _enumset_types
    values = scope.get_enum_values()
    is_remove_all_caps_enum_values = _is_any_not_all_caps_enum_value(values)
    unique = scope.get_unique_enum_values()
    unique_names = [fix_enum_name(n) for n,v,d in unique] # Remove redundant prefix in enum values.
    class_docstring = format_docstring(scope)
    decls = []
    for n,v,d in values:
        n = fix_enum_name(n)
        if is_remove_all_caps_enum_values and _is_all_caps_enum_value(n) and not n in unique_names:
            continue
        ev = ''
        if d:
            ev = '/**\n     * ' + d + '\n     */\n'
        ev += f'    {n}({v}{literal_suffix})'
        decls.append(ev)
    enum_values = ",\n    ".join(decls)
    enum_cases = ";\n        ".join([f'if (value == {v}{literal_suffix}) return {fix_enum_name(n)}' for n,v,_ in unique])
    class_body = f"""
    {enum_values};

    private final {int_type} value;

    {enum_name}({int_type} value) {{
        this.value = value;
    }}

    public {int_type} value() {{
        return this.value;
    }}

    public static {enum_name} fromInt({int_type} value) {{
        {enum_cases};
        return {enum_name}.values()[0];
    }}
"""
    if is_enumset:
        enum_cases = ";\n        ".join([f'if ((value & {v}{literal_suffix}) != 0) enumList.add({fix_enum_name(n)})' for n,v,_ in unique])
        class_body += f"""

    static EnumSet<{enum_name}> setFromInt(int value) {{
        if (value == 0) {{
            return EnumSet.noneOf({enum_name}.class);
        }}
        List<{enum_name}> enumList = new ArrayList<>();
        {enum_cases};
        return EnumSet.copyOf(enumList);
    }}

    static int valueFromSet(EnumSet<{enum_name}> enumSet) {{
        {int_type} tmpvalue = 0;
        for ({enum_name} e: enumSet) {{
            tmpvalue |= e.value;
        }}
        return tmpvalue;
    }}

"""
    class_type = 'enum'
    class_name = enum_name
    full_txt = _apply_template(template, class_body, **locals())
    return full_txt

def is_ctor_valid(child: Scope) -> bool:
    assert child.is_constructor
    """Should this constructor be generated?"""
    scope = child.parent
    args = child.get_args()
    if scope.is_abstract:
        return False
    if child.is_move_constructor or child.is_copy_constructor:
        return False
    else:
        if args:
            t = scope.root.try_find(get_pointee(args[0].canonical_type))
            if t and t.is_record and t.is_abstract:
                # don't create Java version of constructor that takes an interface ptr
                return False
        return True

def format_docstring(scope: Scope, indent='', used_arglist: List[str] = []) -> str:
    # TODO: Fix multiline descriptions
    doc = scope.get_doc()
    if doc:
        lines = doc.summary.splitlines()
        if lines: lines.append('')
        if doc.params:
            for p in doc.params:
                if used_arglist and not p in used_arglist:
                    continue
                lines.append(f'@param {p} {doc.params[p]}')
        if doc.returns:
            lines.append(f'@return {doc.returns}')
        if doc.see:
            lines.append(f'@see {doc.see}')
        doclines = [ indent + '/**' ]
        doclines.extend([ indent + ' * ' + l for l in lines])
        doclines.append(indent + ' */')
        return '\n'.join(doclines)
    else:
        return ''

def make_template_instantiated_name(fullname: str, substituted_template_args: List[str]) -> str:
    # This function uses a pretty primitive scheme for generating the name, but it will suffice for now.
    base_name = strip_prefixes(fullname)
    names = [ create_type_nice_name(strip_prefixes(arg)) for arg in substituted_template_args ]
    namecheck = all([ not n[0].islower() for n in names]) # digits are neither upper or lower case!
    assert namecheck, "Now is the time to make a better naming scheme!"
    if len(names) == 2 and names[1].isdigit():
        # Assume (Float|Int|Double|...)(Vector|Range)(2|3|4|...) etc...
        names.insert(1, base_name)
    else:
        names.insert(0, base_name)
    name = ''.join(names).replace(' ', '')
    name = apply_prefixes(name, fullname)
    return name

def get_template_arg_names(template_args: List[str], template_arg_names: List[str] = None) -> List[str]:
    assert template_args
    if template_arg_names:
#        assert len(template_arg_names) == len(template_args)
        return template_arg_names
    else:
        return [f'type-parameter-0-{i}' for i in range(len(template_args))]

def typename_get_used_template_args(typename: str, template_args: List[str], template_arg_names: List[str] = None) -> List[str]:
    used_template_args = []
    check_names = get_template_arg_names(template_args, template_arg_names)
    check_args = get_template_args(typename)
    for n in check_names:
        if n in check_args:
            used_template_args.append(n)
    return used_template_args

def substitute_template_args(template_args: List[str], args_dict: Dict[str, str]) -> List[str]:
    result: List[str] = []
    for arg in template_args:
        if arg in args_dict:
            result.append(args_dict[arg])
        else:
            result.append(arg)
    return result

def typename_substitute_template_args(typename: str, substituted_template_args: List[str]) -> str:
    if is_templated(typename):
        typename = get_template_name(typename)
        typename = typename + '<' + ', '.join(substituted_template_args) + '>'
    return typename

def get_template_args_dict(instantiate_args: List[str], names: List[str] = None) -> Dict[str, str]:
    def genparam(names: List[str] = None):
        if names:
            it = iter(names)
            while True:
                try:
                    yield next(it)
                except StopIteration:
                    yield '-'
        else:
            i = -1
            while True:
                i += 1
                yield f'type-parameter-0-{i}'
    args_dict = dict(zip(genparam(names), instantiate_args))
    return args_dict

def param_subsitute_template_args(param: Param, instantiate_args: List[str]) -> Param:
    # FIXME! Don't assume 'type-parameter-0-XXX' format. Accept list of names or args dict?
    if instantiate_args and 'type-parameter-0-' in param.canonical_type:
        typename = param.canonical_type
        for i in range(len(instantiate_args)):
            typename = typename.replace(f'type-parameter-0-{i}', instantiate_args[i])
        result = Param(typename, param.name, node=param.typenode)
        result.is_out = param.is_out
        return result
    elif instantiate_args and '<' in param.typename:
        canonical = param.canonical_type
        canonical_template_args = get_template_args(canonical)
        used_template_args = typename_get_used_template_args(param.typename, instantiate_args, canonical_template_args)
        args_dict = get_template_args_dict(instantiate_args, used_template_args)
        substituted_template_args = substitute_template_args(canonical_template_args, args_dict)
        canonical = typename_substitute_template_args(canonical, substituted_template_args)
        result = Param(canonical, param.name, node=param.typenode)
        result.is_out = param.is_out
        return result
    else:
        return param

def arglist_substitute_template_args(arglist: List[Param], template_args: List[str]) -> List[Param]:
    # FIXME! Don't assume 'type-parameter-0-XXX' format. Accept list of names or args dict?
    if template_args:
        newargs = []
        for arg in arglist:
            newargs.append(param_subsitute_template_args(arg, template_args))
        return newargs
    else:
        return arglist

def get_docstring_and_body(function: str) -> Tuple[str, str]:
    if '/**' in function:
        raise NotImplementedError('Handle docstring in overload generation')
    return ('', function)

def create_automatic_overloads(function_signature: str, function_docstring: str, overloads_created: List[str], methods: TextIO) -> None:
    function_name = (re.match('.* (\w+)\(', function_signature) or re.match('(\w+)\(', function_signature))[1]
    for pattern, generator in java_auto_overloads.items():
        if re.match(pattern, function_signature):
            function_return_type = (re.match(f'(.*) {function_name}\(', function_signature) or (['', 'void']))[1]
            function_static = function_return_type.startswith('static ')
            function_return_type = function_return_type.replace('static ', '')
            arglist = re.match('.*\((.*)\)', function_signature)[1].split(', ')
            function_args = { arg.split(' ')[1]: arg.split(' ')[0] for arg in arglist }
            overload = generator(function_name, function_static, function_return_type, function_args)
            if overload:
                doc, function = get_docstring_and_body(overload)
                function = remove_empty_lines(function)
                signature = function.splitlines()[0].replace('public ', '').split(')')[0].strip() + ')'
                if signature not in overloads_created:
                    print(function, file=methods)
                    overloads_created.append(signature)
            return

def create_defaulted_overloads(overload_functions: List[str], overload_default_args: List[Dict[str, str]], function_signature: str, function_docstring: str, overloads_created: List[str], methods: TextIO):
    if not function_signature in overload_functions:
        return
    for i in range(0, len(overload_functions)):
        if overload_functions[i] == function_signature:
            # Get docstring without defaulted arg
            mod_docstring = []
            last_param_line_kept = True
            for docstring_line in function_docstring.splitlines():
                keep_line = True
                if '@param' in docstring_line:
                    for arg in overload_default_args[i]:
                        if arg in docstring_line:
                            keep_line = False
                            break
                    last_param_line_kept = keep_line
                elif '@return' in docstring_line or '*/' in docstring_line:
                    last_param_line_kept = True
                elif not last_param_line_kept:
                    keep_line = False
                if keep_line:
                    mod_docstring.append(docstring_line)
            mod_docstring = '\n'.join(mod_docstring)
            # Get function signature without defaulted arg
            function_sig = overload_functions[i]
            function_sig_arg_string = function_sig[function_sig.find('(')+1:function_sig.find(')')]
            function_sig_arg_tokens = function_sig_arg_string.split(',')
            function_sig_arg_tokens_remove = []
            for token_index, token in enumerate(function_sig_arg_tokens):
                token_tokens = token.strip().split(' ')
                if len(token_tokens) == 2:
                    target_token = token_tokens[1].strip()
                    if target_token in overload_default_args[i]:
                        function_sig_arg_tokens_remove.append(token_index)
            function_sig_arg_tokens_remove.reverse()
            for token_index in function_sig_arg_tokens_remove:
                function_sig_arg_tokens.pop(token_index)
            mod_function_sig_arg_string = ','.join(function_sig_arg_tokens)
            function_sig = function_sig.replace(function_sig_arg_string, mod_function_sig_arg_string)
            if function_sig in overloads_created:
                return
            print(f'\n{mod_docstring}', file=methods)
            # Get function call with defaulted arg set
            function_call = overload_functions[i]
            function_call_arg_string = function_call[function_call.find('(')+1:function_call.find(')')]
            function_call_arg_tokens = function_call_arg_string.split(',')
            mod_function_call_arg_tokens = []
            for token_index, token in enumerate(function_call_arg_tokens):
                token = token.strip()
                token_tokens = token.split(' ')
                if len(token_tokens) == 2:
                    target_token = token_tokens[1].strip()
                    if target_token in overload_default_args[i]:
                        default_arg = f'/*{target_token}=*/{overload_default_args[i][target_token]}'
                        mod_function_call_arg_tokens.append(default_arg)
                    else:
                        mod_function_call_arg_tokens.append(target_token)
                else:
                    mod_function_call_arg_tokens.append(token)
            mod_function_call_arg_string = ', '.join(mod_function_call_arg_tokens)
            function_call = function_call.replace(function_call_arg_string, mod_function_call_arg_string)
            function_call = function_call[function_call.find(' ')+1:] # strip off return type
            ret = '' if 'void' in overload_functions[i] else 'return '
            # Output function
            function    =  f'    public {function_sig} {{\n'
            function    += f'        {ret}{function_call};\n'
            function    += f'    }}'
            overloads_created.append(function_sig)
            print(f'\n{function}', file=methods)

def is_interface_class(typename: str) -> bool:
    return typename in _interfaces

def create_java_class(scope: Scope, template: str, override_name: str = '', template_args: List[str] = [], explicit_children: List[Scope] = []):
    global g_root
    if scope.is_template:
        assert override_name
        assert template_args
    output = io.StringIO()
    methods = io.StringIO()
    class_name = override_name or scope.name
    bases = scope.get_base_classes()
    assert scope.is_record or scope.is_template or scope.is_namespace
    has_instance_methods = False
    has_ctor = False
    overloads_created = []
    overload_functions, overload_default_args, template = get_overloads_from_template(template)
    ignored_nodes = get_ignored_nodes_from_template(template)
    failed_nodes = []
    explicit_add_get_prefix_functions = []
    is_interface = is_interface_class(scope.fullname)
    for regex in _explicit_add_get_prefix:
        if re.match(regex, class_name) != None:
            explicit_add_get_prefix_functions = _explicit_add_get_prefix[regex]
    scopes = [scope]
    if len(bases) > 1:
        # Multiple inheritances is only indirectly supported. Members of base classes other than the first
        # are inlined like normal class methods.
        if not explicit_children:
            scopes.extend([b for b,t in bases[1:]])
            bases = [bases[0]]
    for _scope in scopes:
        children = explicit_children or _scope.get_children()
        for child in children:
            if str(child) in ignored_nodes:
                continue
            try:
                if child.is_anonymous:
                    continue
                if child.is_enum:
                    if inhibit_generation(child):
                        continue
                    if child.get_enum_values():
                        enum_contents = create_java_enum(child, template=load_java_template(child, is_inner_class=True))
                        print(indent(enum_contents), file=methods)
                elif child.is_function or child.is_data_member:
                    if child.nodetype == 'FUNCTION_TEMPLATE':
                        continue
                    if child.is_destructor:
                        continue
                    function_name = child.name
                    jni_function_name = child.overload_name
                    result_param = child.result or Param(child.typename, 'result', is_out=True)
                    if child.is_data_member:
                        function_name = f'get{capfirst(child.name)}'
                        jni_function_name = function_name
                    args = child.get_args()
                    if template_args:
                        result_param = param_subsitute_template_args(result_param, template_args)
                        args = arglist_substitute_template_args(args, template_args)
                    nativeargs = list(args)
                    check_ignore_args(args)
                    check_ignore_args([result_param])
                    if child.is_constructor: 
                        if is_ctor_valid(child):
                            print(create_java_ctor(child, class_name, jni_function_name, True, overloads_created, args), file=methods)
                            has_ctor = True
                            has_instance_methods = True
                    elif function_name.startswith("operator") and _scope is scope:
                        if function_name == "operator==":
                            print(create_java_equals(scope), file=methods)
                            has_instance_methods = True
                    else:
                        if is_bytebuffer_backed(result_param.canonical_type, result_param.typename):
                            nativeargs.insert(0, result_param)
                            native_result_param = Param('void')
                        else:
                            native_result_param = result_param
                        real_return_type = param_to_real_type(result_param, template_args)
                        return_type = cpp_to_java_type(real_return_type)
                        native_return_type = cpp_to_native_type(native_result_param.canonical_type)
                        if is_templated(return_type) and is_templated(real_return_type): # We don't want false positives, like EnumSet<>
                            raise NotImplementedError('Unhandled templated result type')
                        is_function_already_renamed = False
                        if result_param.canonical_type != "void" and not function_name.startswith("Get"):
                            if function_name in explicit_add_get_prefix_functions:
                                function_name = f"get{function_name}"
                                is_function_already_renamed = True
                        if not is_function_already_renamed:
                            # Change function names to camelCase with the following exceptions:
                            # (1) A function with a single letter (e.g. I, J, K, X, Y, Z, etc.)
                            # (2) A function starting with multiple upper-case letters (e.g. IJK, VDS, URL, etc.)
                            function_name_len = len(function_name)
                            change_function_name_to_camel_case = function_name_len > 0
                            if function_name_len == 1:
                                change_function_name_to_camel_case = False
                            elif function_name_len > 1:
                                if function_name[0].isupper() and function_name[1].isupper():
                                    change_function_name_to_camel_case = False
                            if change_function_name_to_camel_case:
                                function_name = function_name[0].lower() + function_name[1:]
                        is_static_method = child.is_static_method or scope.is_namespace
                        static = "static " if is_static_method else ""
                        native_arglist = create_native_arglist(class_name, nativeargs, True, is_static_method)
                        java_arglist = create_java_arglist(args)
                        used_args = []
                        transformed_args, prologue, epilogue = transform_java_functioncall_args(class_name, nativeargs, True, is_static_method, used_arglist=used_args)
                        overload_signature = f'{static}{return_type} {function_name}({java_arglist})'
                        if overload_signature in overloads_created:
                            continue
                        overloads_created.append(overload_signature)
                        if not is_interface:
                            print(f'\n    ///AUTOGEN-OK: {child}', file=methods)
                            print(f'    native private {static}{native_return_type} {jni_function_name}Impl({native_arglist});', file=methods)
                        function_docstring = format_docstring(child, indent='    ', used_arglist=used_args)
                        if function_docstring:
                            print(f'\n{function_docstring}', file=methods)
                        if is_interface:
                            print(f'    public {static}{return_type} {function_name}({java_arglist});', file=methods)
                        else:
                            print(f'    public {static}{return_type} {function_name}({java_arglist}) {{', file=methods)
                            if prologue:
                                print(f'{prologue}', file=methods)
                            return_txt = '' if return_type == 'void' else 'return '
                            if epilogue:
                                print(f'        {jni_function_name}Impl({transformed_args});', file=methods)
                                print(f'{epilogue}', file=methods)
                            else:
                                if is_enumset_type(result_param):
                                    _et = find_enum_type(result_param.canonical_type)
                                    enum_int_type,_,_ = _java_enum_integral_types[_et.enum_integral_type]
                                    _rt = return_type.replace('EnumSet<', '').replace('>', '')
                                    print(f'        {return_txt}{_rt}.setFromInt(({enum_int_type}){jni_function_name}Impl({transformed_args}));', file=methods)
                                elif is_enum_type(result_param):
                                    _et = find_enum_type(result_param.canonical_type)
                                    enum_int_type,_,_ = _java_enum_integral_types[_et.enum_integral_type]
                                    print(f'        {return_txt}{return_type}.fromInt(({enum_int_type}){jni_function_name}Impl({transformed_args}));', file=methods)
                                elif is_pass_by_handle(real_return_type) and not is_marshaled_valuetype(real_return_type):
                                    print(f'        {return_txt}{return_type}.fromNativeObject({jni_function_name}Impl({transformed_args}));', file=methods)
                                else:
                                    print(f'        {return_txt}{jni_function_name}Impl({transformed_args});', file=methods)
                            print('    }', file=methods)
                            if child.is_data_member:
                                property_setter = create_java_property_setter(child, class_name, f'set{capfirst(child.name)}', result_param)
                                if property_setter:
                                    print(property_setter, file=methods)
                            if not is_static_method:
                                has_instance_methods = True
                            create_defaulted_overloads(overload_functions, overload_default_args, overload_signature, function_docstring, overloads_created, methods)
                            create_automatic_overloads(overload_signature, function_docstring, overloads_created, methods)
            except NotImplementedError as e:
                print(f'\n    ///AUTOGEN-FAIL: {child}', file=methods)
                print(f'\nJava: While parsing {child}, the following exception occurred:', file=sys.stderr)
                failed_nodes.append(child)
                print(e, file=sys.stderr)
                print_callstack(e)
                pass
            except TypeIgnored as e:
    #            print(f'\nWhile parsing {child}, the following exception occurred:', file=sys.stderr)
    #            print(e, file=sys.stderr)
    #            print_callstack(e)
                pass
    extra_overloadable_functions, extra_overloadable_function_docstrings, template = get_overloadable_functions_from_template(template)
    for function, docstring in zip(extra_overloadable_functions, extra_overloadable_function_docstrings):
        create_defaulted_overloads(overload_functions, overload_default_args, function, docstring, overloads_created, methods)
    for overload_signature in list(overloads_created):
        function_docstring = '' # FIXME Keep overloads/docstrings in a dict?
        create_automatic_overloads(overload_signature, function_docstring, overloads_created, methods)
    class_implements_interfaces = []   
    _bases = []
    for b, t in bases:
        if is_interface_class(b.fullname):
            class_implements_interfaces.append(b.name)
        else:
            _bases.append((b,t))
    bases = _bases            
    if bases:
        assert len(bases) == 1
        b,t = bases[0]
        if inhibit_generation(b):
            bases = []
        else:
            if b.is_template:
                assert template_args
                basename = strip_prefixes(register_instantiate_with_parameters(f'{b.fullname}{t}', template_args))
                class_extends_txt = f' extends {basename}'
            else:
                class_extends_txt = f' extends {b.name}'
    elif is_interface:
        pass
    elif scope.is_namespace:
        pass
    else:
        bases = [ 'ManagedBase' ]
        class_extends_txt = ' extends ManagedBase'
    is_raii = False
    for raii_class in raii_classes:
        if re.match(raii_class, class_name):
            is_raii = True
            break
    is_raii = False if bases and bases[0] != 'ManagedBase' else is_raii
    if is_raii and has_instance_methods:
        # AutoCloseable interface can be used with the java try-with-resources pattern (RAAI)
        class_implements_interfaces.append('AutoCloseable')
        # create dtor : close() method from AutoCloseable
        dtor    =  f'    public void close() {{\n'
        dtor    += f'        dispose();\n'
        dtor    += f'    }}'
    else:
        dtor = ""
#    if is_interface:
#        print('\n    long getNativeObject();', file=methods)
    if has_instance_methods or bases:
        if bases:
            ctor    =  f'    {class_name}(long nativeobject) {{\n'
            ctor    += f'        super(nativeobject);\n'
            ctor    += f'    }}'
        else:
            print('    private long native_object;', file=output)
            # create package private ctor used by fromNativeObject()
            ctor    =  f'    {class_name}(long nativeobject) {{\n'
            ctor    += f'        if (nativeobject == 0)\n'
            ctor    += f'            throw new IllegalArgumentException("nativeobject handle may not be null");\n'
            ctor    += f'        this.native_object = nativeobject;\n'
            ctor    += f'    }}'
        # construct instance from native object handle
        factory =  f'    static {class_name} fromNativeObject(long nativeobject) {{\n'
        factory += f'        if (nativeobject == 0) {{\n'
        factory += f'           return null;\n'
        factory += f'        }}\n'
        factory += f'        return new {class_name}(nativeobject);\n'
        factory += f'    }}\n'
        factory += f'\n'
        if not bases:
            factory += f'    long getNativeObject() {{\n'
            factory += f'        if (this.isNull())\n'
            factory += f'           throw new RuntimeException("Accessing null object");\n'
            factory += f'        return this.native_object;\n'
            factory += f'    }}'
        print(f"\n{ctor}", file=methods)
        print('    native private long dtorImpl(long nativeobject, boolean isDisposing);\n', file=methods)
        if not 'onDisposing' in ignored_nodes:
            print('    @Override', file=methods)
            print('    protected void onDisposing(long native_object, boolean isDisposing) {', file=methods)
            print('        dtorImpl(native_object, isDisposing);', file=methods)
            print('    }', file=methods)
        if dtor:
            print(f"\n{dtor}", file=methods)
        print(f"\n{factory}", file=methods)
    print(methods.getvalue(), file=output)
    class_docstring = format_docstring(scope)
    class_body = output.getvalue()
    class_type = 'interface' if is_interface else 'class'
    class_implements_txt = '' if not class_implements_interfaces else ' implements ' + ', '.join(class_implements_interfaces)
    full_txt = _apply_template(template, class_body, **locals())
    if failed_nodes:
        print(f'Java: Failed nodes for {class_name}:', file=sys.stderr)
        for n in failed_nodes:
            print(f'///AUTOGEN-FAIL: {n}', file=sys.stderr)
    return full_txt

_IGNORE_GENERATOR_NODES = {
    'void onDisposing(long ': 'onDisposing',
}

def get_ignored_nodes_from_template(template: str):
    nodes = [l.replace('///AUTOGEN-IGNORE: ', '').strip() for l in template.splitlines() if '///AUTOGEN-IGNORE: ' in l]
    for pat in _IGNORE_GENERATOR_NODES:
        if pat in template:
            nodes.append(_IGNORE_GENERATOR_NODES[pat])
    return nodes

def get_overloadable_functions_from_template(template: str):
    template_lines = template.splitlines()
    overloadable_functions = [l.replace('///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: ', '').strip() for l in template_lines if '///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: ' in l]
    overloadable_function_count = len(overloadable_functions)
    overloadable_function_docstrings = [''] * overloadable_function_count
    if overloadable_function_count > 0:
        current_docstringlines = []
        is_in_docstring = False
        for line in template_lines:
            if '/*' in line:
                current_docstringlines.append(line)
                is_in_docstring = True
            elif '*/' in line:
                current_docstringlines.append(line)
                is_in_docstring = False
            elif is_in_docstring:
                current_docstringlines.append(line)
            elif len(line.strip()) == 0:
                pass
            elif 'public' in line:
                for index, overloadable_function in enumerate(overloadable_functions):
                    if overloadable_function in line:
                        overloadable_function_docstrings[index] = '\n'.join(current_docstringlines)
                        current_docstringlines.clear()
                        break
    template_lines = [l for l in template.splitlines() if not '///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: ' in l]
    template = '\n'.join(template_lines)
    template = template + '\n'
    return overloadable_functions, overloadable_function_docstrings, template

def get_overloads_from_template(template: str):
    overload_lines = [l.replace('///AUTOGEN-ADD-OVERLOAD: ', '').strip() for l in template.splitlines() if '///AUTOGEN-ADD-OVERLOAD: ' in l]
    overload_functions = []
    overload_default_args = []
    for l in overload_lines:
        default_args_dict = {}
        tokens = l.split('->')
        if len(tokens) != 2:
            raise ValueError("///AUTOGEN-ADD-OVERLOAD malformed")
        overload_functions.append(tokens[0].strip())
        tokens = tokens[1].strip().split(',')
        if (len(tokens) == 0):
            raise ValueError("///AUTOGEN-ADD-OVERLOAD malformed")
        for token in tokens:
            if '=' not in token:
                raise ValueError("///AUTOGEN-ADD-OVERLOAD malformed")
            default_param_value = token.split('=')
            if len(default_param_value) != 2:
                raise ValueError("///AUTOGEN-ADD-OVERLOAD malformed")
            default_args_dict[default_param_value[0].strip()] = default_param_value[1].strip()
        overload_default_args.append(default_args_dict)
    template_lines = [l for l in template.splitlines() if not '///AUTOGEN-ADD-OVERLOAD: ' in l]
    template = '\n'.join(template_lines)
    template = template + '\n'
    return overload_functions, overload_default_args, template

def process_includes(template: str):
    template_lines = template.splitlines()
    mod_template_lines = []
    for l in template_lines:
        if '///AUTOGEN-INCLUDE: ' in l:
            tokens = l.split(',')
            assert len(tokens) > 1
            include_filename = tokens[0]
            include_filename = include_filename.replace('///AUTOGEN-INCLUDE: ', '').strip()
            tokens.pop(0)
            substitutions = {}
            for substitution_str in tokens:
                substitution_tokens = substitution_str.split('->')
                assert len(substitution_tokens) == 2
                source = substitution_tokens[0].strip()
                target = substitution_tokens[1].strip()
                substitutions[source] = target
            with open(os.path.join(template_dir, include_filename)) as include_file:
                include_file_contents = include_file.read()
                for source in substitutions:
                    include_file_contents = include_file_contents.replace(source, substitutions[source])
                mod_template_lines.extend(include_file_contents.splitlines())
        else:
            mod_template_lines.append(l)
    template = '\n'.join(mod_template_lines)
    template = template + '\n'
    return template

def write_java_file(java_dir: str, name: str, contents: str):
    global g_generated_classes
    javaname = os.path.abspath(os.path.join(java_dir, f'{name}.java'))
    if name in dont_output_list:
        print(f"Skipping '{javaname}'")
    else:
        print(f"Writing '{javaname}'")
        register_generated(name)
        with open(javaname, 'w') as java_file:
            java_file.write(contents)

def load_cpp_template(cls: Scope, alias_name: str = ''):
    cpp_template = load_template(cls, '.cpp', alias_name)
    cpp_template = process_includes(cpp_template)
    return cpp_template

def load_java_template(cls: Scope, is_inner_class: bool=False, alias_name: str = ''):
    java_template = load_template(cls, '.java', alias_name)
    java_template = process_includes(java_template)
    if is_inner_class:
        return java_template
    else:
        global copyright_txt
        global imports_txt
        return copyright_txt + imports_txt + java_template

def load_template(cls: Scope, ext: str, alias_name: str = '') -> str:
    alias_name = alias_name or cls.name

    names = [
        alias_name,
        'Default_Namespace' if cls.is_namespace else cls.name,
        'Default',
    ]
    global template_dir
    for name in names:
        try:
            with open(os.path.join(template_dir, name + ext)) as file:
                contents = file.read()
                if 'long long' in contents:
                    raise ValueError("Invalid template format. Substitute '[u]int64_t' for '[unsigned] long long' parameters.")
                return contents
        except:
            pass
    raise ValueError(f"No template found in '{template_dir}'")

def inhibit_generation(item: Union[Scope, str]) -> bool:
    if isinstance(item, Scope):
        fullname = item.fullname
    else:
        fullname = item
    if fullname in _ignore_types:
        return True
    elif fullname in _marshaled_value_types:
        return True
    elif fullname in _cppjava_typemap:
        return True
    elif fullname in _bytebuffer_backed:
        return True
    else:
        return False

def instantiate_java_class(scope: Scope, template: str, instance_name: str, instance_args: List[str]) -> str:
    javacontents = create_java_class(scope, template, instance_name, instance_args)
    return javacontents

def instantiate_jni_methods(scope: Scope, template: str, instance_name: str, instance_args: List[str]) -> str:
    cppcontents = create_jni_methods(scope, template, instance_name, instance_args)
    return cppcontents

def parse_and_generate(input_header, jni_dir, java_dir):
    global copyright_txt
    global includes_txt
    global g_root
    global g_instantiate_nodes
    header_name = os.path.split(input_header)[1]
    print(f"Parsing '{header_name}'")
    scope = parse_header(input_header, include_dirs, ['JAVA_WRAPPER_GENERATOR', 'OPENVDS_VERSION=""'])
    g_root = scope
    cppfile = io.StringIO()
    header_free_functions = []
    print(f'{copyright_txt}', file=cppfile)
    print(f'{includes_txt}', file=cppfile)
    print(f'#include "{header_name}"\n', file=cppfile)
    print('#ifdef __cplusplus\nextern "C" {\n#endif', file=cppfile)
    for root in [scope.find(ns) for ns in _prefixes if scope.try_find(ns) ]:
        # First, look for type aliases to register
        for item in root.get_children():
            if not os.path.samefile(item.filename, input_header):
                continue
            if include_classes and not item.name in include_classes:
                continue
            if item.name in exclude_classes:
                continue
            if item.is_record:
                if item.is_typealias:
                    if item.is_explicit_instantiation:
                        _template_params = item.get_template_parameters()
                        _template_class = Scope.get_node_fullname(next(item.node.get_children()).canonical)
                        _template_param_list = ', '.join([p for p in _template_params])
                        _alias_name = f'{_template_class}<{_template_param_list}>' 
                        register_typealias(item.fullname, _alias_name)
        # Now process nodes
        for item in root.get_children():
            if not os.path.samefile(item.filename, input_header):
                continue
            if include_classes and not item.name in include_classes:
                continue
            if item.name in exclude_classes:
                continue
            if item.is_record:
                if item.is_typealias:
                    continue
                if inhibit_generation(item):
                    continue
                javacontents = create_java_class(item, template=load_java_template(item, is_inner_class=False))
                write_java_file(java_dir, item.name, javacontents)
                cppcontents = create_jni_methods(item, template=load_cpp_template(item))
                if cppcontents:
                    print(cppcontents, file=cppfile)
            elif item.is_function:
                if not item.is_template and not item.is_class_method and not item.is_operator:
                    header_free_functions.append(item)
            elif item.is_enum:
                if inhibit_generation(item):
                    continue
                if item.get_enum_values():
                    javacontents = create_java_enum(item, template=load_java_template(item, is_inner_class=False))
                    write_java_file(java_dir, item.name, javacontents)
            elif item.kind == 'INVALID':
                pass
            else:
                raise NotImplementedError(f"{item.kind}")
        if header_free_functions:
            if header_name in free_function_header_list:
                ns_class_name,_ = os.path.splitext(header_name)
                ns = root
                javacontents = create_java_class(ns, template=load_java_template(ns, is_inner_class=False, alias_name=ns_class_name), override_name=ns_class_name, explicit_children=header_free_functions)
                write_java_file(java_dir, ns_class_name, javacontents)
                cppcontents = create_jni_methods(ns, template=load_cpp_template(ns, alias_name=ns_class_name), override_name=ns_class_name, explicit_children=header_free_functions)
                if cppcontents:
                    print(cppcontents, file=cppfile)
        while g_instantiate_nodes:
            instantiate_nodes = g_instantiate_nodes
            g_instantiate_nodes = {}
            for name in instantiate_nodes:
                canonical = instantiate_nodes[name]
                class_name = strip_prefixes(name)
                if already_generated(class_name):
                    continue
                assert not '<' in class_name
                template_name = canonical[:canonical.index('<')]
                template_args = canonical[canonical.index('<') + 1: -1].split(', ')
                item = g_root.find(template_name)
                assert item.nodetype == 'CLASS_TEMPLATE' # other kinds not supported atm
                javacontents = instantiate_java_class(item, template=load_java_template(item, is_inner_class=False, alias_name=class_name), instance_name=class_name, instance_args=template_args)
                write_java_file(java_dir, class_name, javacontents)
                cppcontents = instantiate_jni_methods(item, template=load_cpp_template(item, alias_name=class_name), instance_name=class_name, instance_args=template_args)
                if cppcontents:
                    print(cppcontents, file=cppfile)
    jniname = os.path.abspath(os.path.join(jni_dir, os.path.splitext(header_name)[0] + '.cpp'))
    print(f"Writing '{jniname}'")
    print('#ifdef __cplusplus\n}\n#endif', file=cppfile)
    with open(jniname, 'w') as jni_file:
        assert 'long long' not in cppfile.getvalue(), 'The parser should always substitute [u]int64_t for [unsigned] long long.'
        jni_file.write(cppfile.getvalue())

include_dirs = [ '../../src/OpenVDS/OpenVDS', '../../src/OpenVDS' ]

include_classes = [
#    'KnownAxisNames',
]

raii_classes = [
    'VDS$',
    'VolumeDataPageAccessor$',
    'VolumeDataAccessManager$', 
    'VolumeDataRequest$',
    'VolumeData[2-4]D\w*Accessor\w*$',
]

def _create_vdserror_overload(function_name: str, function_static: bool, function_return_type: str, function_args: Dict[str, str]) -> str:
    assert function_static
    callargs = ', '.join([ n for n in function_args])
    assert callargs.endswith(', error')
    function_args.pop('error')
    arglist = ', '.join([ t+' '+n for n, t in function_args.items()])
    get_result = f'{function_return_type} result = ' if not function_return_type == 'void' else ''
    return_result = 'return result;' if not function_return_type == 'void' else ''
    overload = f"""
    public {'static' if function_static else ''} {function_return_type} {function_name}({arglist}) throws java.io.IOException {{
        VDSError error = new VDSError();
        {get_result}{function_name}({callargs});
        if (error.getCode() != 0) {{
            throw new java.io.IOException(error.getString());
        }}
        {return_result}
    }}
    """
    return overload

def _create_ndposarray_overload(function_name: str, function_static: bool, function_return_type: str, function_args: Dict[str, str]) -> str:
    ndposarray_name = ''
    for n, t in function_args.items():
        if t == 'NDPosArray':
            function_args[n] = 'float[]'
            ndposarray_name = n
            break
    assert ndposarray_name
    arglist = ', '.join([ f'{t} {n}' for n, t in function_args.items()])
    callargs = ', '.join([ f'new NDPosArray({n})' if n==ndposarray_name else n for n in function_args])
    overload = f"""
    public {'static' if function_static else ''} {function_return_type} {function_name}({arglist}) {{
        return {function_name}({callargs});
    }}
    """
    return overload

# create overloads for methods matching the following patterns:
java_auto_overloads = {
    '.*NDPosArray samplePositions.*': _create_ndposarray_overload,
    '.*VDS (open|create).*\(.*, VDSError error\)': _create_vdserror_overload,
    '.*void (close|retryableClose)\(.*, VDSError error\)': _create_vdserror_overload,
}

exclude_classes = [
    'MetadataKeyRange',
    'Exception',
    'FatalException',
    'IndexOutOfRangeException',
    'InvalidArgument',
    'InvalidOperation',
    'ReadErrorException',
]

free_function_header_list = [
    'OpenVDS.h',
    'VolumeData.h',
]

header_list = [ 
    'OpenVDS.h',
    'CoordinateTransformer.h',
    'Error.h',
    'Exceptions.h',
    'GlobalMetadataCommon.h',
    'GlobalState.h',
    'IJKCoordinateTransformer.h',
    'KnownMetadata.h',
    'Log.h',
    'MetadataAccess.h',
    'MetadataContainer.h',
    'MetadataKey.h',
    'Optional.h',
    'ValueConversion.h',
    'VolumeData.h',
    'VolumeDataAccess.h',
    'VolumeDataAccessManager.h',
    'VolumeDataAxisDescriptor.h',
    'VolumeDataChannelDescriptor.h',
    'VolumeDataLayout.h',
    'VolumeDataLayoutDescriptor.h',
#   'VolumeIndexer.h',  #Do we need this???
    'VolumeSampler.h',
]

dont_output_list = []

# Write generated cpp files here:
jni_output_dir = '../cpp/src'

# Write generated java files here:
java_output_dir = '../java/src/org/opengroup/openvds'

# Look for headers here_
header_dir = '../../src/OpenVDS/OpenVDS'

# Look for code generator templates here:
template_dir = './javagen_templates'

def main(args):
    global copyright_txt
    global imports_txt
    global includes_txt
    global header_list
    try:
        with open(os.path.join(template_dir, 'Copyright.txt')) as file:
            copyright_txt = file.read()
        with open(os.path.join(template_dir, 'Imports.java')) as file:
            imports_txt = file.read()
        with open(os.path.join(template_dir, 'Includes.cpp')) as file:
            includes_txt = file.read()
    except:
        pass
    if args:
        candidate_headers = [name for name in os.listdir(header_dir) if name in header_list]
        header_list = [header for header in candidate_headers if any([fnmatch.fnmatch(header, pattern) for pattern in args]) ]
    for header in header_list:    
        parse_and_generate(os.path.join(header_dir, header), jni_output_dir, java_output_dir)

if __name__ == "__main__":
    main(sys.argv[1:])
