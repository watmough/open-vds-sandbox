
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

MEMBERS = [
    'X', 'Y', 'Z', 'T'
]

MEMBERS_lower = [ m.lower() for m in MEMBERS ]

def transformTemplate(text: str, typename: str, count: int) -> str:
    return text.replace("TYPENAME", typename).replace("VECTORCOUNT", str(count)).replace("JAVATYPE", JAVATYPES[typename]).replace("PRIMITIVETYPE", TYPEMAP[typename])
             
def createDefaultCtor(typename: str, count: int) -> str:
    init = ", ".join(["(PRIMITIVETYPE)0" for i in range(count)])
    contents = """
    public TYPENAMEVectorVECTORCOUNT() {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init)
    return transformTemplate(contents, typename, count)

def createFullCtor(typename: str, count: int) -> str:
    args = ", ".join([ f"PRIMITIVETYPE {MEMBERS_lower[c]}" for c in range(0, count) ])
    init = ", ".join([ f"{MEMBERS_lower[c]}" for c in range(0, count) ])
    contents = """
    public TYPENAMEVectorVECTORCOUNT(ARGS) {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init).replace("ARGS", args)
    return transformTemplate(contents, typename, count)

def createCopyCtor(typename: str, count: int) -> str:
    init = ", ".join([ f"rhs.get{MEMBERS[c]}()" for c in range(0, count) ])
    contents = """
    public TYPENAMEVectorVECTORCOUNT(TYPENAMEVectorVECTORCOUNT rhs) {
        this.createByteBuffer(JAVATYPE.BYTES * VECTORCOUNT);
        this.set(INIT);
    }
""".replace("INIT", init)
    return transformTemplate(contents, typename, count)

def createEquals(typename: str, count: int) -> str:
    contents = """
    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        TYPENAMEVectorVECTORCOUNT real_other = (TYPENAMEVectorVECTORCOUNT)other;
        """.replace("TYPENAME", typename).replace("VECTORCOUNT", str(count))
    compare = """ &&
                """.join([ f"this.get{MEMBERS[c]}() == real_other.get{MEMBERS[c]}()" for c in range(0, count) ])
    return contents + f"return ({compare});\n    }}\n"

def createSetter(typename: str, count: int) -> str:
    setters = "\n        ".join([f"        this.getByteBufferProxy().putTYPENAME({c} * JAVATYPE.BYTES, {MEMBERS_lower[c]});" for c in range(0, count)])
    args = ", ".join([f"PRIMITIVETYPE {MEMBERS_lower[c]}" for c in range(0, count)])
    contents = """
    public void set(ARGS) {
        SETTERS
    }
""".replace("SETTERS", setters).replace("ARGS", args)
    return transformTemplate(contents, typename, count)
    
def createSetters(typename: str, count: int) -> str:
    contents = "\n".join([f"""
    public void set{MEMBERS[c]}(PRIMITIVETYPE value) {{
        this.getByteBufferProxy().putTYPENAME({c} * JAVATYPE.BYTES, value);
    }}\n""" for c in range(0, count)])
    return transformTemplate(contents, typename, count)

def createGetters(typename: str, count: int) -> str:
    contents = "\n".join([f"""
    public PRIMITIVETYPE get{MEMBERS[c]}() {{
        return this.getByteBufferProxy().getTYPENAME({c} * JAVATYPE.BYTES);
    }}\n""" for c in range(0, count)])

    return transformTemplate(contents, typename, count)

def createToString(typename: str, count: int) -> str:
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
    return transformTemplate(contents, typename, count)

HANDLERS = [
    createDefaultCtor,
    createFullCtor,
    createCopyCtor,
    createEquals,
    createSetter,
    createSetters,
    createGetters,
    createToString,
]

for n in range(2, 5):
    for t in TYPEMAP:
        valuetype = TYPEMAP[t]
        content = COPYRIGHT + """
package org.opengroup.openvds;

public class TYPENAMEVectorVECTORCOUNT extends ByteBufferBackedObject {
"""
        for handler in HANDLERS:
            content += handler(t, n)
        content += "}\n"
        content = transformTemplate(content, t, n)
        class_filename = transformTemplate("TYPENAMEVectorVECTORCOUNT.java", t, n)
        with open(f'../src/org/opengroup/openvds/{class_filename}', 'w') as file:
            print(content, file=file)
