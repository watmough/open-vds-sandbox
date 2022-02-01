
COPYRIGHT = """
/*
 * Copyright 2021 The Open Group
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

RANGE_MEMBERS = [
    'min', 'max'
]

VECTOR_MEMBERS = [
    'x', 'y', 'z', 't'
]

MEMBERS = None

def transformTemplate(text: str, class_name: str, typename: str, count: int) -> str:
    return text.replace("CLASSNAME", class_name).replace("TYPENAME", typename).replace("VECTORCOUNT", str(count)).replace("JAVATYPE", JAVATYPES[typename]).replace("PRIMITIVETYPE", TYPEMAP[typename])
             
def createDefaultCtor(class_name: str, typename: str, count: int) -> str:
    init = ", ".join(["(PRIMITIVETYPE)0" for i in range(count)])
    contents = """
    public CLASSNAME() {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init)
    return transformTemplate(contents, class_name, typename, count)

def createFullCtor(class_name: str, typename: str, count: int) -> str:
    args = ", ".join([ f"PRIMITIVETYPE {MEMBERS[c]}" for c in range(0, count) ])
    init = ", ".join([ f"{MEMBERS[c]}" for c in range(0, count) ])
    contents = """
    public CLASSNAME(ARGS) {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init).replace("ARGS", args)
    return transformTemplate(contents, class_name, typename, count)

def createByteBufferCtor(class_name: str, typename: str, count: int) -> str:
    contents = """
    public CLASSNAME(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, JAVATYPE.BYTES * VECTORCOUNT);
    }
"""
    return transformTemplate(contents, class_name, typename, count)

def createCopyCtor(class_name: str, typename: str, count: int) -> str:
    init = ", ".join([ f"rhs.get{MEMBERS[c].capitalize()}()" for c in range(0, count) ])
    contents = """
    public CLASSNAME(CLASSNAME rhs) {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init)
    return transformTemplate(contents, class_name, typename, count)

def createEquals(class_name: str, typename: str, count: int) -> str:
    contents = """
    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        CLASSNAME real_other = (CLASSNAME)other;
        """.replace("TYPENAME", typename).replace("VECTORCOUNT", str(count))
    compare = """ &&
                """.join([ f"this.get{MEMBERS[c].capitalize()}() == real_other.get{MEMBERS[c].capitalize()}()" for c in range(0, count) ])
    return contents + f"return ({compare});\n    }}\n"

def createSetter(class_name: str, typename: str, count: int) -> str:
    setters = "\n    ".join([f"    this.getByteBufferProxy().putTYPENAME({c} * JAVATYPE.BYTES, {MEMBERS[c]});" for c in range(0, count)])
    args = ", ".join([f"PRIMITIVETYPE {MEMBERS[c]}" for c in range(0, count)])
    contents = """
    public void set(ARGS) {
    SETTERS
    }
""".replace("SETTERS", setters).replace("ARGS", args)
    return transformTemplate(contents, class_name, typename, count)
    
def createSetters(class_name: str, typename: str, count: int) -> str:
    contents = "\n".join([f"""
    public void set{MEMBERS[c].capitalize()}(PRIMITIVETYPE value) {{
        this.getByteBufferProxy().putTYPENAME({c} * JAVATYPE.BYTES, value);
    }}\n""" for c in range(0, count)])
    return transformTemplate(contents, class_name, typename, count)

def createGetters(class_name: str, typename: str, count: int) -> str:
    contents = "\n".join([f"""
    public PRIMITIVETYPE get{MEMBERS[c].capitalize()}() {{
        return this.getByteBufferProxy().getTYPENAME({c} * JAVATYPE.BYTES);
    }}\n""" for c in range(0, count)])

    return transformTemplate(contents, class_name, typename, count)

def createToString(class_name: str, typename: str, count: int) -> str:
    contents = f"""
    public String toString() {{
        String value = "(";
        for (int i = 0; i < {count}; ++i)
        {{
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getTYPENAME(i * JAVATYPE.BYTES);
        }}
        value = value + ")";
        return value;
    }}
"""   
    return transformTemplate(contents, class_name, typename, count)

HANDLERS = [
    createDefaultCtor,
    createFullCtor,
    createByteBufferCtor,
    createCopyCtor,
    createEquals,
    createSetter,
    createSetters,
    createGetters,
    createToString,
]

def make_classes(class_name: str, members: list, index_seq: range):
    global MEMBERS 
    MEMBERS = members
    for n in index_seq:
        for t in TYPEMAP:
            valuetype = TYPEMAP[t]
            content = COPYRIGHT + """
package org.opengroup.openvds;
import java.nio.*;

public class CLASSNAME extends ByteBufferBackedObject {
"""
            for handler in HANDLERS:
                content += handler(class_name, t, n)
            content += "}\n"
            content = transformTemplate(content, class_name, t, n)
            class_filename = transformTemplate("CLASSNAME.java", class_name, t, n)
            with open(f'../src/org/opengroup/openvds/{class_filename}', 'w') as file:
                print(content, file=file)
    
make_classes("TYPENAMERange", RANGE_MEMBERS, range(2,3))
make_classes("TYPENAMEVectorVECTORCOUNT", VECTOR_MEMBERS, range(2,5))
