
COPYRIGHT = """
/*
 * Copyright 2022 The Open Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
"""

# Create vector + vector composite classes with ByteBuffer backing
 
TYPEMAP = {
    'Int':    'int',
    'Float':  'float',
    'Double': 'double',
}

JAVATYPES = {
    'Int':    'Integer',
    'Float':  'Float',
    'Double': 'Double',
}

MEMBERS = None

def capfirst(value: str) -> str:
    return value[0].upper() + value[1:]

def transformTemplate(text: str, class_name: str, typename: str, count: int, composite_count: int) -> str:
    return text.replace("CLASSNAME", class_name).replace("TYPENAME", typename).replace("VECTORCOUNT", str(count)).replace("JAVATYPE", JAVATYPES[typename]).replace("PRIMITIVETYPE", TYPEMAP[typename]).replace("COMPOSITECOUNT", str(composite_count))

def createDefaultBytesize(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = """
    public static final int BYTES = JAVATYPE.BYTES * VECTORCOUNT * COMPOSITECOUNT;
"""
    return transformTemplate(contents, class_name, typename, count, composite_count)
             
def createDefaultCtor(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = """
    public CLASSNAME() {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT * COMPOSITECOUNT);
    }
"""
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createToArray(class_name: str, typename: str, count: int, composite_count: int) -> str:
    init = ", ".join([ f"get{capfirst(MEMBERS[c])}()" for c in range(0, count) ])
    contents = f"""
    public PRIMITIVETYPE[] toArray() {{
        return new PRIMITIVETYPE[]{{ {init} }};
    }}
"""    
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createArrayCtor(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = """
    public CLASSNAME(PRIMITIVETYPE[] array) {
        if (array == null) {
            throw new NullPointerException("array may not be null.");
        }
        if (array.length != VECTORCOUNT) {
            throw new IllegalArgumentException("array must be of length VECTORCOUNT. ");
        }
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT * COMPOSITECOUNT);
        this.getManagedBuffer().put(array);
    }
"""
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createFullCtor(class_name: str, typename: str, count: int, composite_count: int) -> str:
    args = ", ".join([ f"PRIMITIVETYPE {MEMBERS[c]}" for c in range(0, count) ])
    init = ", ".join([ f"{MEMBERS[c]}" for c in range(0, count) ])
    contents = """
    public CLASSNAME(ARGS) {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init).replace("ARGS", args)
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createByteBufferCtor(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = """
    public CLASSNAME(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, JAVATYPE.BYTES * VECTORCOUNT);
    }
"""
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createCopyCtor(class_name: str, typename: str, count: int, composite_count: int) -> str:
    init = ", ".join([ f"rhs.get{capfirst(MEMBERS[c])}()" for c in range(0, count) ])
    contents = """
    public CLASSNAME(CLASSNAME rhs) {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init)
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createEquals(class_name: str, typename: str, count: int, composite_count: int) -> str:
    assert composite_count == 1
    contents = """
    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        CLASSNAME real_other = (CLASSNAME)other;
        """.replace("TYPENAME", typename).replace("VECTORCOUNT", str(count))
    compare = """ &&
                """.join([ f"this.get{capfirst(MEMBERS[c])}() == real_other.get{capfirst(MEMBERS[c])}()" for c in range(0, count) ])
    return contents + f"return ({compare});\n    }}\n"

def createSetter(class_name: str, typename: str, count: int, composite_count: int) -> str:
    setters = "\n    ".join([f"    this.getManagedBuffer().putTYPENAME({c} * JAVATYPE.BYTES, {MEMBERS[c]});" for c in range(0, count)])
    args = ", ".join([f"PRIMITIVETYPE {MEMBERS[c]}" for c in range(0, count)])
    contents = """
    public void set(ARGS) {
    SETTERS
    }
""".replace("SETTERS", setters).replace("ARGS", args)
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createPutter(class_name: str, typename: str, count: int, composite_count: int) -> str:
    setters = "\n    ".join([f"    managedbuffer.putTYPENAME({c} * JAVATYPE.BYTES + byteoffset, this.get{capfirst(MEMBERS[c])}());" for c in range(0, count)])
    contents = """
    void put(ManagedBuffer managedbuffer, int byteoffset) {
    SETTERS
    }
""".replace("SETTERS", setters)
    return transformTemplate(contents, class_name, typename, count, composite_count)
    
def createSetters(class_name: str, typename: str, count: int, composite_count) -> str:
    contents = "\n".join([f"""
    public void set{capfirst(MEMBERS[c])}(PRIMITIVETYPE value) {{
        this.getManagedBuffer().putTYPENAME({c} * JAVATYPE.BYTES, value);
    }}\n""" for c in range(0, count)])
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createGetters(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = "\n".join([f"""
    public PRIMITIVETYPE get{capfirst(MEMBERS[c])}() {{
        return this.getManagedBuffer().getTYPENAME({c} * JAVATYPE.BYTES);
    }}\n""" for c in range(0, count)])

    return transformTemplate(contents, class_name, typename, count, composite_count)

def createToString(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = f"""
    public String toString() {{
        String value = "(";
        for (int i = 0; i < {count}; ++i)
        {{
            if (i > 0)
                value = value + ", ";
            value = value + this.getManagedBuffer().getTYPENAME(i * JAVATYPE.BYTES);
        }}
        value = value + ")";
        return value;
    }}
"""   
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createCompositeGetters(class_name: str, typename: str, count: int, composite_count: int) -> str:
    vectortype = f'{typename}Vector{count}'
    contents = "\n".join([f"""
    public PRIMITIVETYPE get{capfirst(MEMBERS[c])}() {{
        return new PRIMITIVETYPE(this.getBackingByteBuffer(), this.getByteBufferOffset() + {typename}.BYTES * {count} * {c});
    }}\n""".replace("PRIMITIVETYPE", vectortype) for c in range(0, composite_count)])
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createCompositeSetters(class_name: str, typename: str, count: int, composite_count: int) -> str:
    vectortype = f'{typename}Vector{count}'
    
    contents = "\n".join([f"""
    public void set{capfirst(MEMBERS[c])}(PRIMITIVETYPE value) {{
          this.get{capfirst(MEMBERS[c])}().put(this.getManagedBuffer(), this.getByteBufferOffset() + {typename}.BYTES * {count} * {c});
    }}\n""".replace("PRIMITIVETYPE", vectortype) for c in range(0, composite_count)])
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createCompositeToString(class_name: str, typename: str, count: int, composite_count: int) -> str:
    values = "\n".join([f'            value = value + "{capfirst(MEMBERS[c])}(" + this.get{capfirst(MEMBERS[c])}().toString() + ")";' for c in range(0, composite_count)])
    contents = f"""
    public String toString() {{
        String value = "(";
        for (int i = 0; i < {count}; ++i)
        {{
            if (i > 0)
                value = value + ", ";
{values}
        }}
        value = value + ")";
        return value;
    }}
"""   
    return transformTemplate(contents, class_name, typename, count, composite_count)

def createCompositeEquals(class_name: str, typename: str, count: int, composite_count: int) -> str:
    contents = """
    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        CLASSNAME real_other = (CLASSNAME)other;
        """.replace("TYPENAME", typename).replace("VECTORCOUNT", str(count))
    compare = """ &&
                """.join([ f"this.get{capfirst(MEMBERS[c])}().equals(real_other.get{capfirst(MEMBERS[c])}())" for c in range(0, composite_count) ])
    return contents + f"return ({compare});\n    }}\n"

def createMatrixDefaultCtor(class_name: str, typename: str, count: int, composite_count: int) -> str:
    init = "\n".join([f'        getManagedBuffer().put{typename}(this.getByteBufferOffset() + {typename}Vector{count}.BYTES * {c} + {typename}.BYTES * {str(c)}, 1.0);' for c in range(0, composite_count)])
    contents = f"""
    public CLASSNAME() {{
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT * COMPOSITECOUNT);
{init}        
    }}
"""
    return transformTemplate(contents, class_name, typename, count, composite_count)

RANGE_MEMBERS = [
    'min', 'max'
]

VECTOR_MEMBERS = [
    'x', 'y', 'z', 't'
]

NDPOS_MEMBERS = [
    'pos0', 'pos1', 'pos2', 'pos3', 'pos4', 'pos5', 
]

VECTOR_HANDLERS = [
    createDefaultCtor,
    createFullCtor,
    createByteBufferCtor,
    createCopyCtor,
    createArrayCtor,
    createEquals,
    createPutter,
    createSetter,
    createSetters,
    createGetters,
    createToString,
    createToArray,
    createDefaultBytesize,
]

IJKGRID_HANDLERS = [
    createDefaultCtor,
    createCompositeGetters,
    createCompositeSetters,
    createCompositeToString,
    createCompositeEquals,
    createDefaultBytesize,
]

IJKGRID_MEMBERS = [ 'Origin', 'IUnitStep', 'JUnitStep', 'KUnitStep' ]

MATRIX_HANDLERS = [
    createMatrixDefaultCtor,
    createCompositeGetters,
    createCompositeSetters,
    createCompositeToString,
    createCompositeEquals,
    createDefaultBytesize,
]

MATRIX_MEMBERS = [ 'x', 'y', 'z', 't' ]

def make_class(class_name: str, members: list, handlers: list, member_type: str, vector_count: int, composite_count: int):
    global MEMBERS 
    MEMBERS = members
    valuetype = TYPEMAP[member_type]
    content = COPYRIGHT + """
package org.opengroup.openvds;
import java.nio.*;

// THIS FILE WAS AUTOGENERATED BY 'make-vector-classes.py' DO NOT EDIT!

public class CLASSNAME extends ByteBufferBackedObject {
"""
    for handler in handlers:
        content += handler(class_name, member_type, vector_count, composite_count)
    content += "}\n"
    content = transformTemplate(content, class_name, member_type, vector_count, composite_count)
    class_filename = transformTemplate("CLASSNAME.java", class_name, member_type, vector_count, composite_count)
    with open(f'../src/org/opengroup/openvds/{class_filename}', 'w') as file:
        print(content, file=file)

def make_vector_classes(class_name: str, members: list, index_seq: range):
    for n in index_seq:
        for t in TYPEMAP:
            make_class(class_name, members, VECTOR_HANDLERS, t, n, 1)

def make_composite_class(class_name: str, members: list, handlers: list, member_type: str, member_vector_count: int, composite_count: int = 0):
    make_class(class_name, members, handlers, member_type, member_vector_count, composite_count if composite_count > 0 else len(members))


def main():
    make_vector_classes("TYPENAMERange", RANGE_MEMBERS, range(2,3))
    make_vector_classes("TYPENAMEVectorVECTORCOUNT", VECTOR_MEMBERS, range(2,5))
    make_class("NDPos", NDPOS_MEMBERS, VECTOR_HANDLERS, "Float", 6, 1)

    make_composite_class('IJKGridDefinition', IJKGRID_MEMBERS, IJKGRID_HANDLERS, 'Double', 3)
    make_composite_class('TYPENAMEMatrix3x3', MATRIX_MEMBERS, MATRIX_HANDLERS, 'Double', 3, 3)
    make_composite_class('TYPENAMEMatrix4x4', MATRIX_MEMBERS, MATRIX_HANDLERS, 'Double', 4, 4)

if __name__ == "__main__":
    main()