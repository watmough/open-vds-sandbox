#### OpenVDS Java API

Since OpenVDS version 2.3 the Java API has undergone a major change; The API source code is now 
mainly auto-generated from the C++ API header files. This should lower the burden of maintaining
an up-to-date version of the Java API.

Several classes have added support for `try-with-resources` by implementing the `AutoCloseable`
interface. This allows for more predictable lifecycle management of objects that are in
a rather close-knit dependence hierarchy. It also allows for more deterministic memory
management, particularly in the case of the `VolumeDataAccessor` and `VolumeDataPageAccessor`
classes as memory allocated by these classes will be immediatly free'd upon calling `close` on
an instance. This happens implicitly when using the `try-with-resources` construct.

Instead of arrays, the API now uses strictly typed vector classes for n-tuples of floats, ints etc.
These are named FloatVector3, FloatVector4 and so on.

Methods operating on bulk data now only use direct ByteBuffers, no arrays. Some of the request-methods 
have overloads that allocate their own data buffers for convenience.

The pre-existing helper classes `InMemoryVDSGenerator`, `AzureVDSGenerator` have been renamed for consistency.

The `VDSHandle` class is superseded by the `VDS` class.

The `BufferUtils` and `Cleaner` classes have been removed because of simplified memory management.

##### Java samples

The standalone code samples in `java/java/demo` have been updated. `CreateVDS.java` and `SliceDump.java` in 
particular, should be helpful in understanding how to read and write VDSs.

##### Regenerating and augmenting the Java API

The Java API is generated from the C++ headers by the `javagen.py` script located in the 
`java/tools` folder. To run it, you need to have Python with the [clang](https://pypi.org/project/clang/) 
package installed. To run the script from the command line, use the following command:

`$ python javagen.py`

The script parses a list of header files and creates wrapper classes for the classes and 
enum types defined in each header. Methods may be added or supressed from within code generator 
template files in `java/tools/javagen_templates`. 

As the C++ API evolves, the Java generator may produce warnings like 

`///AUTOGEN-FAIL: CXX_METHOD GetCurrentUploadError void (const char **, int *, const char **) FUNCTIONPROTO`

When the generator looks at function parameters and tries to to create semantically valid Java and C++ code,
it will sometimes fail, especially when raw pointers are involved. When this happens, there are four options:
- Ignore the warning. This is not recommended.
- Suppress the warning in a template file, e.g. `///AUTOGEN-IGNORE: CXX_METHOD GetCurrentUploadError void (const char **, int *, const char **) FUNCTIONPROTO`
- Suppress the warning and provide an implementation.
- Modify `javagen.py` to handle the case on a general basis.
