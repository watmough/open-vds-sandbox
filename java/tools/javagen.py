import sys
import io
import os
import traceback
import re
from typing import Tuple, List, Dict, Set
from clang.cindex import Cursor, Type
from more_itertools import peekable
from more_itertools.more import exactly_n
from parse_cpp_header import parse_header, Param, Scope, ScopeDoc

_print_exception_call_stacks = False # For debugging
_functioncall_readability_threshold = 2 # Method formatting: if function argument count is greater than this, arguments will be spread across multiple lines

copyright_txt = ''
imports_txt = ''
includes_txt = ''

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

_ignore_types = [
    "OpenVDS::Error",
    "OpenVDS::StringWrapper",
#    "Hue::HueSpaceLib::MetadataKeyRange",
#    "Hue::HueSpaceLib::IHueObj",
#    "Hue::HueSpaceLib::IVolumeDataAccess",
#    "Hue::HueSpaceLib::IVolumeDataAccessManager",
#    "Hue::HueSpaceLib::IVolumeDataAccessor", 
#    "Hue::HueSpaceLib::IVolumeDataReadWriteAccessor",
#    "Hue::HueSpaceLib::IVolumeDataReadAccessor",
#    "Hue::HueSpaceLib::IVolumeDataReadWriteAccessor",
#    "Hue::HueSpaceLib::IHasVolumeDataAccess",
#    "OpenVDS::VDSIJKGridDefinition",
#    "Hue::HueSpaceLib::VolumeDataCacheItem",
#    "Hue::HueSpaceLib::VolumeDataRequest::RequestFormat",
#    "CUstream_st",
]

_marshaled_value_types = [
#    "Hue::HueSpaceLib::ProxyBLOB",
#    "Hue::HueSpaceLib::VolumeDataChannelDescriptor",
#    "Hue::HueSpaceLib::VolumeDataLayoutDescriptor",
#    "Hue::HueSpaceLib::VolumeDataAxisDescriptor",
#    "Hue::Util::IJKGridDefinition",
]

_prefixes = [ "OpenVDS" ]

_explicit_add_get_prefix = {
#    "IJKCoordinateTransformer$" : ["IJKGrid", "IJKSize", "IJKToVoxelDimensionMap", "IJKToWorldTransform", "WorldToIJKTransform", "IJKAnnotationStart", "IJKAnnotationEnd", "AnnotationsDefined"],
#    "VolumeDataRequest$" : ["RequestID", "BufferByteSize", "BufferDataType"],
#    "VolumeData[2-4]D\w*Accessor\w*$" : ["RegionCount", "Region", "RegionFromIndex", "CurrentRegion"]
}

_cppjava_typemap = {
    "std::basic_string<char, std::char_traits<char>, std::allocator<char>>": "String",
    "const char *":                                 "String",
    "std::string":                                  "String",
#    "Hue::HueSpaceLib::ProxyBLOB":                  "byte[]",
#    "Hue::HueSpaceLib::VolumeDataFormat":           "VCVoxelFormat",
#    "Hue::HueSpaceLib::VolumeDataComponents":       "VCVoxelComponents",
#    "Hue::HueSpaceLib::VolumeDataAxisDescriptor":   "VolumeDataAxisDescriptor",
#    "Hue::HueSpaceLib::ReadErrorException":         "ReadErrorException",
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
    "std::basic_string<char, std::char_traits<char>, std::allocator<char>>": "jstring",
#    "Hue::HueSpaceLib::ProxyBLOB": "jbyteArray",
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
    "uint16_t":             "jcharArray",
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
}

_smart_ptr_types = [
    'std::shared_ptr',
    'std::unique_ptr',
]

_vector_types = [
    'std::vector',
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

def is_licenseapi(class_name: str) -> bool:
    return class_name == 'HueLicenseAPIHelper'

def is_marshaled_valuetype(typename: str) -> bool:
    return clean_typename(typename) in _marshaled_value_types

def is_bytebuffer_backed(typename: str) -> bool:
    return clean_typename(typename) in _bytebuffer_backed

def is_smartptr_type(typename: str) -> bool:
    return any([typename.startswith(s) for s in _smart_ptr_types])

def is_vector_type(typename: str) -> bool:
    return any([typename.startswith(s) for s in _vector_types])

def is_pass_by_handle(typename: str) -> bool:
    clean_name = clean_typename(typename)
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

def get_template_args(templated_name: str) -> str:
    assert is_templated(templated_name)
    return templated_name[templated_name.index('<')+1 : templated_name.rindex('>')]

def get_template_arg0(templated_name: str) -> str:
    return get_template_args(templated_name).split(',') [0] # nasty

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

def clean_typename(typename: str) -> str:
    return typename.replace("const", "").replace("*", "").replace("&", "").strip()

def create_type_nice_name(typename: str) -> str:
    if typename[0].islower():
        return typename[0].upper() + typename[1:]
    else:
        return typename

def make_template_instantiated_name(base_name: str, template_args: List[str]) -> str:
    # This function uses a pretty primitive scheme for generating the name, but it will suffice for now.
    names = [ create_type_nice_name(strip_prefixes(typename_substitute_template_args(arg, template_args))) for arg in template_args ]
    namecheck = all([ n[0].isupper() for n in names])
    assert namecheck, "Now is the time to make a better naming scheme!"
    names.insert(0, base_name)
    name = ''.join(names).replace(' ', '')
    return name

def already_generated(class_name: str):
    global g_generated_classes
    return strip_prefixes(class_name) in g_generated_classes

def register_generated(class_name: str):
    global g_generated_classes
    assert not already_generated(class_name)
    g_generated_classes.append(class_name)

def register_typealias(alias: str, canonical_type: str):
    global g_canonical_to_alias
    global g_alias_to_canonical
    global g_instantiate_nodes
    assert 'long long' not in canonical_type
    if not canonical_type in g_canonical_to_alias:
        g_canonical_to_alias[canonical_type] = alias
    if alias in g_alias_to_canonical:
        assert g_alias_to_canonical[alias] == canonical_type
    else:
        g_alias_to_canonical[alias] = canonical_type
    if not already_generated(alias) and alias not in g_instantiate_nodes:
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

def register_instantiate(node: Type, template_args: List[str]) -> str:
    assert isinstance(node, Type)
    global g_instantiate_nodes
    global g_generated_classes
    typename = node.spelling
    if template_args:
        name = node.get_canonical().spelling
        canonical = Scope.get_node_fullname(node, strip_prefixes(name))
        assert '<' in canonical
        if 'type-parameter-' in canonical:
            assert '<' in typename
            used_template_args = typename_get_used_template_args(canonical, template_args)
            canonical = typename_substitute_template_args(canonical, template_args)
            typename = make_template_instantiated_name(get_template_name(canonical), used_template_args)
            register_typealias(typename, canonical)
        else:
            assert not '<' in typename
            register_typealias(typename, canonical)
    return typename

def register_instantiate_with_parameters(canonical: str, template_args: List[str]) -> str:
    assert isinstance(canonical, str)
    global g_instantiate_nodes
    global g_canonical_to_alias
    used_template_args = typename_get_used_template_args(canonical, template_args)
    canonical = typename_substitute_template_args(canonical, template_args)
    if canonical in g_canonical_to_alias:
        typename = g_canonical_to_alias[canonical]
    else:
        typename = make_template_instantiated_name(get_template_name(canonical), used_template_args)
        register_typealias(typename, canonical)
    return typename

def param_to_real_type(param: Param, template_args: List[str]) -> str:
    if is_templated(param.canonical_type):
        if not is_smartptr_type(param.canonical_type):
            if not 'std' in param.canonical_type: # eeh..
                return register_instantiate(param.typenode, template_args)
            return param.typename
    if is_smartptr_type(param.canonical_type):
        return param.typename
    return param.canonical_type

def _cpp_to_java_type(typename: str) -> str:
    if find_enum_type(typename):
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
        if not element_type in _cppjni_array_typemap:
            raise NotImplementedError(f"Unsupported vector type: {element_type}")
        java_element_type = cpp_to_java_type(element_type)
        return f"{java_element_type}[]"
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

def cpp_to_java_type(typename: str) -> str:
    j = _cpp_to_java_type(typename)
    if '::' in j:
        return strip_prefixes(clean_typename(j)).replace('::', '.')
    else:
        return j

def cpp_to_native_type(typename: str) -> str:
    assert typename != 'jlong'
    if find_enum_type(typename):
        return "int"
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
        if not element_type in _cppjni_array_typemap:
            raise NotImplementedError(f"Unsupported vector type: {element_type}")
        java_element_type = cpp_to_native_type(element_type)
        return f"{java_element_type}[]"
    elif is_bytebuffer_backed(typename):
        return "ByteBuffer"
    elif is_optional_type(typename):
        assert False, "We should never get here"
    elif is_pass_by_handle(typename):
        return "long"
    else:
        return cpp_to_java_type(typename)
        
def cpp_to_jni_type(typename: str) -> str:
    assert typename != 'jlong'
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
        if not element_type in _cppjni_array_typemap:
            raise NotImplementedError(f"Unsupported vector type: {element_type}")
        return _cppjni_array_typemap[element_type]
    elif is_bytebuffer_backed(typename):
        assert False, "We should never get here"
    elif is_optional_type(typename):
        assert False, "We should never get here"
    elif find_enum_type(typename):
        return "jint"
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
    if is_include_proxyinterface_arg and not is_licenseapi(class_name):
        arglist.append('jobject jproxyinterface')

    if is_static_method:
        pass
    else:
        arglist.append('jlong native_handle')
    p = peekable(args)
    for arg in p:
        type_, name = arg.canonical_type, arg.name
        if is_bytebuffer_backed(type_):
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
            jnitype_ = cpp_to_jni_type(type_)
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
        assert is_bytebuffer_backed(result.canonical_type) # This is the only way we should get here.
        epilogue.append(f'*({typename}*)((char*)env->GetDirectBufferAddress({name}bytebuffer) + {name}byteoffset) = result;')
    for arg in p:
        type_, name = arg.canonical_type, arg.name
        if is_array(type_):
            arr_element_type = get_array_element(type_)
            arr_size = get_array_size(type_)
            arr_addr = '&' if '*' in type_ else ''
            is_mutable = 'false'
            if not 'const' in type_:
                is_mutable = 'true'
            prologue.append(f"auto tmp{name} = HueJNIArrayAdapter<{arr_element_type},{arr_size},{is_mutable}>(env, {name});")
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
            arglist.append(f"HueJNIByteBufferAdapter<{cleantype}>(env, {name}bytebuffer, {name}byteoffset)")
        elif is_optional_type(type_):
            # optional<T> values are passed as 2 separate parameters: the T value and whether the T value is valid.
            t = get_template_arg0(type_)
            arglist.append(f"use_{name} ? Hue::HueSpaceLib::optional<{t}>({name}) : Hue::HueSpaceLib::optional<{t}>()")
        elif use_buffer_protocol([arg, p.peek(None)]):
            # get size parameter from bytebuffer object
            elem_type = get_pointee(type_)
            prologue.append(f"auto tmp{name} = HueJNIAsyncBuffer<{elem_type}>(env, {name});")
            arglist.append(f"tmp{name}.buffer(), tmp{name}.byteSize()")
            if 'const' in type_:
                # Buffer is read-only, so its lifetime is not assumed to be longer than the duration of this call,
                # so no need to keep it alive by adding a global reference.
                pass
            else:
                # Keep it alive!
                epilogue.append(f"context->registerGlobalRef(env, {name});")
            next(p) # consume buffer size parameter
        else:
            jnitype_ = cpp_to_jni_type(type_)
            if jnitype_ == "jstring":
                if is_static_method:
                    arglist.append(f"HueJNIStringWrapper(env, {name})")
                else:
                    arglist.append(f"HueJNIStringWrapper(env, native_handle, {name})")
            elif jnitype_ == "jboolean":
                arglist.append(f"{name} ? true : false")
            elif is_enum_type(arg):
                if is_scoped_enum_type(arg):
                    arglist.append(f"(enum {type_}){name}")
                else:
                    arglist.append(f"({type_}){name}")
            elif is_pass_by_handle(type_):
                cpptype = clean_typename(type_)
                if '&' in type_:
                    arglist.append(f"*HueJNI_cast<{cpptype}>({name})")
                else:
                    arglist.append(f"HueJNI_cast<{cpptype}>({name})")
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
    if class_name == 'HueLicenseAPIHelper':
        proto = "\nJNIEXPORT {} JNICALL Java_com_hue_licenseapi_{}_{}Impl\n  ({})".format(return_type, class_name, method_name, args)
    else:
        proto = "\nJNIEXPORT {} JNICALL Java_com_hue_proxylib_{}_{}Impl\n  ({})".format(return_type, class_name, method_name, args)
    return proto

def _override_fullname(scope: Scope, class_name: str) -> str:
    fullname = '::'.join([scope.prefix, class_name])
    return fullname

def create_jni_ctor(scope: Scope, ctor: Scope, class_name: str, class_canonical_name: str, overload_name: str, extra_args: List[Param]=[]) -> str:
    class_fullname = _override_fullname(scope, class_name)
    overload_name = overload_name.replace(ctor.name, "ctor")
    proto = create_jni_proto(Param(class_fullname), class_name, overload_name, extra_args=extra_args, is_include_proxyinterface_arg=True, is_static_method=True)
    args, prologue, epilogue = transform_jni_functioncall_args(extra_args, is_static_method=True)
    body = f"""
{{
  JEnvPushPop
    stackitem(env, jproxyinterface);

  HUE_JNI_TRY
  {{
    auto context = new HueJNIObjectContext_t<{class_canonical_name}>();
{prologue}
    auto native_handle = context->handle();
    context->setObject(new {class_canonical_name}({args}));
{epilogue}
    return native_handle;
  }}
  HUE_JNI_CATCH
  return 0;
}}
"""
    return proto + body

def create_jni_dtor(scope: Scope, class_name: str, class_canonical_name: str) -> str:
    proto = create_jni_proto(Param('void'), class_name, "dtor", is_include_proxyinterface_arg=False)
    body = f"""
{{
  HUE_JNI_TRY
  {{
    HueJNI_destroyHandle<{class_canonical_name}>(env, native_handle);
  }}
  HUE_JNI_CATCH
}}
"""
    return proto + body

def create_jni_equals(class_name, class_canonical_name):
    proto = create_jni_proto(Param('bool'), class_name, "operatorEQ", extra_args=[Param(typename=class_canonical_name, name='other_native_handle')], is_include_proxyinterface_arg=False, is_static_method=False)
    body = f"""
{{
  HUE_JNI_TRY
  {{
    auto pInstance = HueJNI_cast<{class_canonical_name}>(native_handle);
    auto pOtherInstance = HueJNI_cast<{class_canonical_name}>(other_native_handle);
    return *pInstance == *pOtherInstance;
  }}
  HUE_JNI_CATCH
  return 0;
}}
"""
    return proto + body

def find_enum_type(real_return_type: str) -> Scope:
    result = g_root.try_find(real_return_type)
    return result if result and result.is_enum else None

def is_enum_type(arg: Param) -> bool:
    assert isinstance(arg, Param)
    return True if find_enum_type(arg.canonical_type) else False

def is_scoped_enum_type(arg: Param) -> bool:
    t = find_enum_type(arg.canonical_type)
    if t:
        if t.parent.is_record:
            return True
    return False
    
def create_jni_methods(scope: Scope, template: str, override_name: str = '', template_args: List[str] = []):
    output = io.StringIO()
    class_name = override_name or scope.name
    classname_full = _override_fullname(scope, class_name)
    if template_args:
        assert not '<' in scope.fullname
        class_canonical_name = f'{scope.fullname}<' + ', '.join(template_args) + '>'
    else:
        class_canonical_name = classname_full
    children = scope.get_children()
    has_ctor = False
    has_instance_methods = False
    ignored_nodes = get_ignored_nodes_from_template(template)
    failed_nodes = []
    for child in children:
        if str(child) in ignored_nodes:
            continue
        local_output = io.StringIO()
        try:
            if child.is_function: # and not scope.is_abstract: 
                if child.is_template:
                    continue
                if child.is_destructor:
                    continue
                result_param = child.result
                args = child.get_args()
                if template_args:
                    result_param = param_subsitute_template_args(result_param, template_args)
                    args = arglist_substitute_template_args(args, template_args)
                if is_bytebuffer_backed(result_param.canonical_type):
                    args.insert(0, result_param)
                    result_param = Param(typename='void', is_out=True)
                    real_return_type = result_param.canonical_type
                check_ignore_args(args)
                check_ignore_args([result_param])
                real_return_type = result_param.canonical_type
                return_type = cpp_to_jni_type(result_param.canonical_type)
                method_name = child.name
                overload_name = child.overload_name
                if method_name.startswith("operator"):
                    if method_name == "operator==":
                        method = create_jni_equals(class_name, class_canonical_name)
                        print(method, file=local_output)
                        has_instance_methods = True
                elif child.is_constructor:
                    if is_ctor_valid(child):
                        print(create_jni_ctor(scope, child, class_name, class_canonical_name, overload_name, args), file=local_output)
                        has_instance_methods = True
                        has_ctor = True
                else:
                    islicenseapi = class_name == 'HueLicenseAPIHelper'
                    proto = create_jni_proto(result_param, class_name, overload_name, extra_args=args, is_include_proxyinterface_arg=True, is_static_method=child.is_static_method)
                    print(proto, file=local_output)
                    declare_result = 'auto result = ' if not child.result.canonical_type == 'void' else ''
                    if is_ref_type(real_return_type) and is_pass_by_handle(real_return_type):
                        declare_result = 'auto& result = '
                    retval = ''
                    print("{", file=local_output)
                    if not islicenseapi:
                        print("  JEnvPushPop\n    stackitem(env, jproxyinterface);\n", file=local_output)
                        print("  HUE_JNI_TRY\n  {", file=local_output)
                    invoke_args, prologue, epilogue = transform_jni_functioncall_args(args, is_static_method=child.is_static_method)
                    if prologue:
                        print(prologue, file=local_output)
                    if child.is_static_method:
                        print("    {}{}({});".format(declare_result, child.fullname, invoke_args), file=local_output)
                    else:
                        print("    auto pInstance = HueJNI_cast<{}>(native_handle);".format(class_canonical_name), file=local_output)
                        print("    {}pInstance->{}({});".format(declare_result, child.name, invoke_args), file=local_output)
                    if return_type == 'jstring':
                        retval = 'HueJNI_newString(env, result)'
                    elif is_marshaled_valuetype(real_return_type):
                        marshaled_type = cpp_to_jni_type(real_return_type)
                        print(f'    {marshaled_type} real_result = {marshaled_type}();', file=local_output)
                        print(f'    Marshaling::Convert(real_result, result);', file=local_output)
                        retval = 'real_result'
                    elif is_vector_type(real_return_type):
                        element_type = get_template_arg0(real_return_type)
                        retval = f'HueJNIVectorAdapter<{element_type}>(env, result).toArray()'
                    elif is_pass_by_handle(real_return_type):
                        if is_enum_type(result_param):
                            retval = '(jint)result'
                        elif is_smartptr_type(real_return_type):
                            retval = 'HueJNI_createObjectContext(result)'.format(real_return_type)
                        elif is_ptr_type(real_return_type):
                            # This handle will not destroy the backing object because it is not the owner:
                            retval = 'HueJNI_createNonOwningObjectContext(result)'
                        elif is_ref_type(real_return_type):
                            # This handle will not destroy the backing object because it is not the owner:
                            retval = 'HueJNI_createNonOwningObjectContext(&result)'
                        else:
                            retval = 'HueJNI_createObjectContext(new {}(result))'.format(clean_typename(real_return_type))
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
                    if not islicenseapi:
                        print("  }\n  HUE_JNI_CATCH", file=local_output)
                    if not return_type == "void" and not islicenseapi:
                        print("  return 0;", file=local_output)
                    print("}", file=local_output)
                    if not child.is_static_method:
                        has_instance_methods = True
        except NotImplementedError as e:
            print(f'///AUTOGEN-FAIL: {child}', file=output)
            failed_nodes.append(child)
            print("\nC++: While parsing {}, the following exception occurred:".format(child))
            print(e, file=sys.stderr)
            print_callstack(e)
        except TypeIgnored as e:
#            print(f'\nWhile parsing {child}, the following exception occurred:', file=sys.stderr)
#            print(e, file=sys.stderr)
#            print_callstack(e)
            pass
        else:
            output.write(local_output.getvalue())
    if has_instance_methods:
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
#    if is_include_proxyinterface_arg and not is_licenseapi(class_name):
#        arglist.append('ProxyInterface proxyInterface')
    if not is_static_method:
        arglist.append('long native_object')
    p = peekable(args)
    for arg in p:
        type_, name_ = arg.canonical_type, arg.name
        if use_buffer_protocol([arg, p.peek(None)]):
            arglist.append(f'ByteBuffer {name_}')
            next(p)
        elif is_optional_type(type_):
            otype = get_template_arg0(type_)
            arglist.append(f'{otype} {name_}, boolean use_{name_}')
        elif is_bytebuffer_backed(type_):
            arglist.append(f'ByteBuffer {name_}, long {name_}_byteoffset')
        else:
            nativetype_ = cpp_to_native_type(type_)
            arglist.append(nativetype_ + " " + name_)
    return ", ".join(arglist)        

def create_java_arglist(args: List[Param]) -> str:
    arglist = []
    p = peekable(args)
    for arg in p:
        type_, name_ = arg.canonical_type, arg.name
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
            nativetype_ = cpp_to_java_type(type_)
            arglist.append(f'{nativetype_} {name_}')
    return ", ".join(arglist)        

def transform_java_functioncall_args(class_name: str, args: List[Param], is_include_proxyinterface_arg: bool=False, is_static_method: bool=False, used_arglist = []) -> Tuple[str, str, str]:
    arglist = []
#    if is_include_proxyinterface_arg and not is_licenseapi(class_name):
#        arglist.append('ProxyInterface.getInstanceSafe()')
    if not is_static_method:
        arglist.append('getNativeObject()')
    prologue = []
    epilogue = []
    p = peekable(args)

    # Make sure parameter for any explicitly sized array arg is the right length
    for arg in p:
        type_, name_ = arg.canonical_type, arg.name
        if is_array(type_):
            array_size = get_array_size(type_)
            if array_size > 0:
                prologue.append(f'if ({name_}.length != {array_size}) throw new IllegalArgumentException("Array \\"{name_}\\" must have length {array_size}");')

    p = peekable(args)
    if args and p.peek().is_out:
        result: Param = next(p)
        typename = cpp_to_java_type(result.typename)  #clean_typename(result.typename)
        name_ = result.name
        assert is_bytebuffer_backed(result.canonical_type) # This is the only way we should get here.
        prologue.append(f'{typename} {name_} = new {typename}();')
        arglist.append(f'{name_}.getBackingByteBuffer(), {name_}.getByteBufferOffset()')
        epilogue.append(f'return {name_};')
    for arg in p:
        type_, name_ = arg.canonical_type, arg.name
        used_arglist.append(name_)
        if is_enum_type(arg):
            arglist.append(f'{name_}.value()')
        elif is_marshaled_valuetype(type_):
            arglist.append(name_)
        elif is_bytebuffer_backed(type_):
            arglist.append(f'{name_}.getBackingByteBuffer(), {name_}.getByteBufferOffset()')
        elif use_buffer_protocol([arg, p.peek(None)]):
            arglist.append(f'{name_}')
            next(p)
        elif is_optional_type(type_):
            otype = get_template_arg0(type_)
            arglist.append(f'{name_} == null ? ({otype})0 : ({otype}){name_}, {name_} != null')
        elif is_pass_by_handle(type_):
            arglist.append(f"{name_}.getNativeObject()")
        else:
            arglist.append(name_)
    _args = ", ".join(arglist)
    _prologue = "\n".join(['        ' + p for p in prologue])
    _epilogue = "\n".join(['        ' + e for e in epilogue])
    return _args, _prologue, _epilogue

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

def create_java_enum(scope, template):
    enum_name = scope.name
    values = scope.get_enum_values()
    is_remove_all_caps_enum_values = _is_any_not_all_caps_enum_value(values)
    unique = scope.get_unique_enum_values()
    unique_names = [n for n,v,d in unique]
    class_docstring = format_docstring(scope)
    decls = []
    for n,v,d in values:
        if is_remove_all_caps_enum_values and _is_all_caps_enum_value(n) and not n in unique_names:
            continue
        ev = ''
        if d:
            ev = '/**\n     * ' + d + '\n     */\n'
        ev += f'    {n}({v})'
        decls.append(ev)
    enum_values = ",\n    ".join(decls)
    enum_cases = ";\n        ".join([f'case {v}: return {n}' for n,v,_ in unique])
    class_body = f"""
    {enum_values};

    private final int value;

    {enum_name}(int value) {{
        this.value = value;
    }}

    public int value() {{
        return this.value;
    }}

    public static {enum_name} fromInt(int value) {{
        switch(value) {{
        {enum_cases};
        }}
        return {enum_name}.values()[0];
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

def typename_get_used_template_args(typename: str, template_args: List[str]) -> str:
    used_template_args = []
    for i in range(len(template_args)):
        if f'type-parameter-0-{i}' in typename:
            used_template_args.append(template_args[i])
    return used_template_args

def typename_substitute_template_args(typename: str, template_args: List[str]) -> str:
    for i in range(len(template_args)):
        typename = typename.replace(f'type-parameter-0-{i}', template_args[i])
    return typename

def param_subsitute_template_args(param: Param, template_args: List[str]) -> Param:
    if template_args and 'type-parameter-0-' in param.canonical_type:
        typename = param.canonical_type
        for i in range(len(template_args)):
            typename = typename.replace(f'type-parameter-0-{i}', template_args[i])
        result = Param(typename, param.name, node=param.typenode)
        result.is_out = param.is_out
        return result
    else:
        return param

def arglist_substitute_template_args(arglist: List[Param], template_args: List[str]) -> List[Param]:
    if template_args:
        newargs = []
        for arg in arglist:
            newargs.append(param_subsitute_template_args(arg, template_args))
        return newargs
    else:
        return arglist

def create_defaulted_overloads(overload_functions, overload_default_args, function_signature, function_docstring, methods):
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
            print(f'\n{mod_docstring}', file=methods)
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
            print(f'\n{function}', file=methods)

def create_java_class(scope: Scope, template: str, override_name: str = '', template_args: List[str] = []):
    global g_root
    if scope.is_template:
        assert override_name
        assert template_args
    output = io.StringIO()
    methods = io.StringIO()
    class_name = override_name or scope.name
    bases = scope.get_base_classes()
    assert scope.is_record or scope.is_template
    has_instance_methods = False
    has_ctor = False
    children = scope.get_children()
    overloads_created = []
    overload_functions, overload_default_args, template = get_overloads_from_template(template)
    ignored_nodes = get_ignored_nodes_from_template(template)
    failed_nodes = []
    explicit_add_get_prefix_functions = []
    for regex in _explicit_add_get_prefix:
        if re.match(regex, class_name) != None:
            explicit_add_get_prefix_functions = _explicit_add_get_prefix[regex]
    for child in children:
        if str(child) in ignored_nodes:
            continue
        try:
            if child.is_anonymous:
                continue
            if child.is_enum:
                if dont_generate_wrapper_for(child):
                    continue
                if child.get_enum_values():
                    enum_contents = create_java_enum(child, template=load_java_template(child, is_inner_class=True))
                    print(indent(enum_contents), file=methods)
            elif child.is_function:
                if child.nodetype == 'FUNCTION_TEMPLATE':
                    continue
                if child.is_destructor:
                    continue
                function_name = child.name
                jni_function_name = child.overload_name
                result_param: Param = child.result
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
                elif function_name.startswith("operator"):
                    if function_name == "operator==":
                        print(create_java_equals(scope), file=methods)
                        has_instance_methods = True
                else:
                    if is_bytebuffer_backed(result_param.canonical_type):
                        nativeargs.insert(0, result_param)
                        native_result_param = Param('void')
                    else:
                        native_result_param = result_param
                    real_return_type = param_to_real_type(result_param, template_args)
                    return_type = cpp_to_java_type(real_return_type)
                    native_return_type = cpp_to_native_type(native_result_param.canonical_type)
                    if is_templated(return_type):
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
                    static = "static " if child.is_static_method else ""
                    native_arglist = create_native_arglist(class_name, nativeargs, True, child.is_static_method)
                    java_arglist = create_java_arglist(args)
                    used_args = []
                    transformed_args, prologue, epilogue = transform_java_functioncall_args(class_name, nativeargs, True, child.is_static_method, used_arglist=used_args)
                    overload_signature = f'{static}{return_type} {function_name}({java_arglist})'
                    if overload_signature in overloads_created:
                        continue
                    overloads_created.append(overload_signature)
                    print(f'\n    ///AUTOGEN-OK: {child}', file=methods)
                    print(f'    native private {static}{native_return_type} {jni_function_name}Impl({native_arglist});', file=methods)
                    function_docstring = format_docstring(child, indent='    ', used_arglist=used_args)
                    if function_docstring:
                        print(f'\n{function_docstring}', file=methods)
                    print(f'    public {static}{return_type} {function_name}({java_arglist}) {{', file=methods)
                    if prologue:
                        print(f'{prologue}', file=methods)
                    return_txt = '' if return_type == 'void' else 'return '
                    if epilogue:
                        print(f'        {jni_function_name}Impl({transformed_args});', file=methods)
                        print(f'{epilogue}', file=methods)
                    else:
                        if is_enum_type(result_param):
                            print(f'        {return_txt}{return_type}.fromInt({jni_function_name}Impl({transformed_args}));', file=methods)
                        elif is_pass_by_handle(real_return_type) and not is_marshaled_valuetype(real_return_type):
                            print(f'        {return_txt}{return_type}.fromNativeObject({jni_function_name}Impl({transformed_args}));', file=methods)
                        else:
                            print(f'        {return_txt}{jni_function_name}Impl({transformed_args});', file=methods)
                    if not child.is_static_method:
                        has_instance_methods = True
                    print('    }', file=methods)
                    create_defaulted_overloads(overload_functions, overload_default_args, overload_signature, function_docstring, methods)
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
        create_defaulted_overloads(overload_functions, overload_default_args, function, docstring, methods)
    if bases:
        assert len(bases) == 1
        b,t = bases[0]
        if dont_generate_wrapper_for(b):
            bases = []
        else:
            if b.is_template:
                assert template_args
                basename = strip_prefixes(register_instantiate_with_parameters(f'{b.fullname}{t}', template_args))
                class_extends_txt = f' extends {basename}'
            else:
                class_extends_txt = f' extends {b.name}'
    else:
        bases = [ 'ManagedBase' ]
        class_extends_txt = ' extends ManagedBase'
    is_raii = False
    for raii_class in raii_classes:
        if re.match(raii_class, class_name):
            is_raii = True
            break
    is_raii = False if bases else is_raii
    if is_raii and has_instance_methods:
        # AutoCloseable interface can be used with the java try-with-resources pattern (RAAI)
        class_implements_txt = ' implements AutoCloseable'
        # create dtor : close() method from AutoCloseable
        dtor    =  f'    public void close() {{\n'
        dtor    += f'        dispose();\n'
        dtor    += f'    }}'
    else:
        dtor = ""
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
            ctor    += f'            throw new NullPointerException("nativeobject");\n'
            ctor    += f'        this.native_object = nativeobject;\n'
            ctor    += f'    }}'

        # construct instance from native object handle
        factory =  f'    static {class_name} fromNativeObject(long nativeobject) {{\n'
        factory += f'        return new {class_name}(nativeobject);\n'
        factory += f'    }}\n'
        factory += f'\n'

        if not bases:
            factory += f'    long getNativeObject() {{\n'
            factory += f'        if (this.native_object == 0)\n'
            factory += f'           throw new RuntimeException("Accessing disposed object");\n'
            factory += f'        return this.native_object;\n'
            factory += f'    }}'
        
        print(f"\n{ctor}", file=methods)
        print('    native private long dtorImpl(long nativeobject);\n', file=methods)
        print('    @Override', file=methods)
        print('    protected void onDisposing(long native_object) {', file=methods)
        print('        dtorImpl(native_object);', file=methods)
        print('    }', file=methods)
        if dtor:
            print(f"\n{dtor}", file=methods)
        print(f"\n{factory}", file=methods)
    print(methods.getvalue(), file=output)
    class_docstring = format_docstring(scope)
    class_body = output.getvalue()
    full_txt = _apply_template(template, class_body, **locals())
    if failed_nodes:
        print(f'Java: Failed nodes for {class_name}:', file=sys.stderr)
        for n in failed_nodes:
            print(f'///AUTOGEN-FAIL: {n}', file=sys.stderr)
    return full_txt

def get_ignored_nodes_from_template(template: str):
    nodes = [l.replace('///AUTOGEN-IGNORE: ', '').strip() for l in template.splitlines() if '///AUTOGEN-IGNORE: ' in l]
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
        cls.name,
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

def dont_generate_wrapper_for(item: Scope) -> bool:
    if item.fullname in _ignore_types:
        return True
    elif item.fullname in _marshaled_value_types:
        return True
    elif item.fullname in _cppjava_typemap:
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
    scope = parse_header(input_header, include_dirs)
    g_root = scope
    cppfile = io.StringIO()
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
                        register_typealias(item.fullname, Scope.type_get_canonical_name(item.node.type))
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
                if dont_generate_wrapper_for(item):
                    continue
                javacontents = create_java_class(item, template=load_java_template(item, is_inner_class=False))
                write_java_file(java_dir, item.name, javacontents)
                cppcontents = create_jni_methods(item, template=load_cpp_template(item))
                if cppcontents:
                    print(cppcontents, file=cppfile)
            elif item.is_function:
                pass
            elif item.is_enum:
                if dont_generate_wrapper_for(item):
                    continue
                if item.get_enum_values():
                    javacontents = create_java_enum(item, template=load_java_template(item, is_inner_class=False))
                    write_java_file(java_dir, item.name, javacontents)
            elif item.kind == 'INVALID':
                pass
            else:
                raise NotImplementedError(f"{item.kind}")
        while g_instantiate_nodes:
            instantiate_nodes = g_instantiate_nodes
            g_instantiate_nodes = {}
            for name in instantiate_nodes:
                canonical = instantiate_nodes[name]
                class_name = strip_prefixes(name)
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
    'VolumeDataAccessManager$', 
    'VolumeDataRequest$',
    'VolumeData[2-4]D\w*Accessor\w*$',
]

exclude_classes = [
    'MetadataKeyRange',
]

jni_output_dir = '../cpp'
header_dir = '../../src/OpenVDS/OpenVDS'
dont_output_list = []

header_list = [ 
    'KnownMetadata.h',
#    'MetadataAccess.h',
    'MetadataKey.h',
    'GlobalState.h',
    'OpenVDS.h',
#    'ProcessorPreference.h',
#    'VolumeDataAccess.h',
#    'IJKCoordinateTransformer.h',
#    'VolumeData.h',
#    'ErrorCode.h',
#    'VolumeDataAccessManager.h',
]

template_dir = './javagen_templates'
java_output_dir = '../src/org/opengroup/openvds'

def main():
    global copyright_txt
    global imports_txt
    global includes_txt
    try:
        with open(os.path.join(template_dir, 'Copyright.txt')) as file:
            copyright_txt = file.read()
        with open(os.path.join(template_dir, 'Imports.java')) as file:
            imports_txt = file.read()
        with open(os.path.join(template_dir, 'Includes.cpp')) as file:
            includes_txt = file.read()
    except:
        pass
    for header in header_list:    
        parse_and_generate(os.path.join(header_dir, header), jni_output_dir, java_output_dir)

if __name__ == "__main__":
    main()
