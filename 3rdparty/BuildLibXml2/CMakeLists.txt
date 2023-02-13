include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(CheckSymbolExists)

set (SRC_DIR "${libxml2_SOURCE_DIR}")

set(LIBXML2_WITH_C14N OFF)
set(LIBXML2_WITH_CATALOG OFF)
set(LIBXML2_WITH_DEBUG OFF)
set(LIBXML2_WITH_DOCB OFF)
set(LIBXML2_WITH_EXPR OFF)
set(LIBXML2_WITH_FTP OFF)
set(LIBXML2_WITH_HTML OFF)
set(LIBXML2_WITH_HTTP OFF)
set(LIBXML2_WITH_ICOFFV OFF)
set(LIBXML2_WITH_ICU OFF)
set(LIBXML2_WITH_ISO8859X OFF)
set(LIBXML2_WITH_LEGACY OFF)
set(LIBXML2_WITH_LZMA OFF)
set(LIBXML2_WITH_MEM_DEBUG OFF)
set(LIBXML2_WITH_MODULES OFF)
set(LIBXML2_WITH_OUTPUT ON)
set(LIBXML2_WITH_PATTERN OFF)
set(LIBXML2_WITH_PROGRAMS OFF)
set(LIBXML2_WITH_PUSH ON)
set(LIBXML2_WITH_PYTHOFF OFF)
set(LIBXML2_WITH_READER ON)
set(LIBXML2_WITH_REGEXPS OFF)
set(LIBXML2_WITH_RUN_DEBUG OFF)
set(LIBXML2_WITH_SAX1 OFF)
set(LIBXML2_WITH_SCHEMAS OFF)
set(LIBXML2_WITH_SCHEMATROFF OFF)
set(LIBXML2_WITH_TESTS OFF)
set(LIBXML2_WITH_THREADS ON)
set(LIBXML2_WITH_THREAD_ALLOC OFF)
set(LIBXML2_WITH_TREE ON)
set(LIBXML2_WITH_TRIO OFF)
set(LIBXML2_WITH_UNICODE OFF)
set(LIBXML2_WITH_VALID OFF)
set(LIBXML2_WITH_WRITER ON)
set(LIBXML2_WITH_XINCLUDE OFF)
set(LIBXML2_WITH_XPATH OFF)
set(LIBXML2_WITH_XPTR OFF)
set(LIBXML2_WITH_ZLIB OFF)
set(LIBXML2_XMLCONF_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Working directory for XML Conformance Test Suite")
set(LIBXML_MAJOR_VERSION 2)
set(LIBXML_MINOR_VERSION 9)
set(LIBXML_MICRO_VERSION 10)
set(LIBXML_VERSION_NUMBER ${LIBXML_MAJOR_VERSION}0${LIBXML_MINOR_VERSION}0${LIBXML_MICRO_VERSION})

foreach(VARIABLE IN ITEMS WITH_AUTOMATA WITH_C14N WITH_CATALOG WITH_DEBUG WITH_DOCB WITH_EXPR WITH_FTP WITH_HTML WITH_HTTP WITH_ICONV WITH_ICU WITH_ISO8859X WITH_LEGACY WITH_LZMA WITH_MEM_DEBUG WITH_MODULES WITH_OUTPUT WITH_PATTERN WITH_PUSH WITH_READER WITH_REGEXPS WITH_RUN_DEBUG WITH_SAX1 WITH_SCHEMAS WITH_SCHEMATRON WITH_THREADS WITH_THREAD_ALLOC WITH_TREE WITH_TRIO WITH_UNICODE WITH_VALID WITH_WRITER WITH_XINCLUDE WITH_XPATH WITH_XPTR WITH_ZLIB)
	if(LIBXML2_${VARIABLE})
		set(${VARIABLE} 1)
	else()
		set(${VARIABLE} 0)
	endif()
endforeach()

if(MSVC)
	configure_file(${SRC_DIR}/include/win32config.h config.h COPYONLY)
else()
	check_c_source_compiles("
		void __attribute__((destructor))
		f(void) {}
		int main(void) { return 0; }
	" ATTRIBUTE_DESTRUCTOR)
	check_c_source_compiles("
		#include <netdb.h>
		int main() { (void) gethostbyname((const char*) \"\"); return 0; }
	" GETHOSTBYNAME_ARG_CAST_CONST)
	if(NOT GETHOSTBYNAME_ARG_CAST_CONST)
		set(GETHOSTBYNAME_ARG_CAST "(char *)")
	else()
		set(GETHOSTBYNAME_ARG_CAST "/**/")
	endif()
	check_include_files(arpa/inet.h HAVE_ARPA_INET_H)
	check_include_files(arpa/nameser.h HAVE_ARPA_NAMESER_H)
	check_struct_has_member("struct sockaddr_storage" ss_family "sys/socket.h;sys/types.h" HAVE_SS_FAMILY)
	check_struct_has_member("struct sockaddr_storage" __ss_family "sys/socket.h;sys/types.h" HAVE_BROKEN_SS_FAMILY)
	if(HAVE_BROKEN_SS_FAMILY)
		set(ss_family __ss_family)
	endif()
	check_function_exists(class HAVE_CLASS)
	check_include_files(ctype.h HAVE_CTYPE_H)
	check_include_files(dirent.h HAVE_DIRENT_H)
	check_include_files(dlfcn.h HAVE_DLFCN_H)
	check_library_exists(dl dlopen "" HAVE_DLOPEN)
	check_include_files(dl.h HAVE_DL_H)
	check_include_files(errno.h HAVE_ERRNO_H)
	check_include_files(fcntl.h HAVE_FCNTL_H)
	check_function_exists(finite HAVE_FINITE)
	check_include_files(float.h HAVE_FLOAT_H)
	check_function_exists(fpclass HAVE_FPCLASS)
	check_function_exists(fprintf HAVE_FPRINTF)
	check_function_exists(fp_class HAVE_FP_CLASS)
	check_function_exists(ftime HAVE_FTIME)
	check_function_exists(getaddrinfo HAVE_GETADDRINFO)
	check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
	check_include_files(inttypes.h HAVE_INTTYPES_H)
	check_function_exists(isascii HAVE_ISASCII)
	check_function_exists(isinf HAVE_ISINF)
	check_function_exists(isnan HAVE_ISNAN)
	check_function_exists(isnand HAVE_ISNAND)
	check_library_exists(history append_history "" HAVE_LIBHISTORY)
	check_library_exists(lzma lzma_code "" HAVE_LIBLZMA)
	check_library_exists(pthread pthread_join "" HAVE_LIBPTHREAD)
	check_library_exists(readline readline "" HAVE_LIBREADLINE)
	check_library_exists(z gzread "" HAVE_LIBZ)
	check_include_files(limits.h HAVE_LIMITS_H)
	check_function_exists(localtime HAVE_LOCALTIME)
	check_include_files(lzma.h HAVE_LZMA_H)
	check_include_files(malloc.h HAVE_MALLOC_H)
	check_include_files(math.h HAVE_MATH_H)
	check_include_files(memory.h HAVE_MEMORY_H)
	check_function_exists(mmap HAVE_MMAP)
	check_function_exists(munmap HAVE_MUNMAP)
	check_symbol_exists(DIR ndir.h HAVE_NDIR_H)
	check_include_files(netdb.h HAVE_NETDB_H)
	check_include_files(netinet/in.h HAVE_NETINET_IN_H)
	check_include_files(poll.h HAVE_POLL_H)
	check_function_exists(printf HAVE_PRINTF)
	check_include_files(pthread.h HAVE_PTHREAD_H)
	check_function_exists(putenv HAVE_PUTENV)
	check_function_exists(rand HAVE_RAND)
	check_function_exists(rand_r HAVE_RAND_R)
	check_include_files(resolv.h HAVE_RESOLV_H)
	check_library_exists(dld shl_load "" HAVE_SHLLOAD)
	check_function_exists(signal HAVE_SIGNAL)
	check_include_files(signal.h HAVE_SIGNAL_H)
	check_function_exists(snprintf HAVE_SNPRINTF)
	check_function_exists(sprintf HAVE_SPRINTF)
	check_function_exists(srand HAVE_SRAND)
	check_function_exists(sscanf HAVE_SSCANF)
	check_function_exists(stat HAVE_STAT)
	check_include_files(stdarg.h HAVE_STDARG_H)
	check_include_files(stdint.h HAVE_STDINT_H)
	check_include_files(stdlib.h HAVE_STDLIB_H)
	check_function_exists(strftime HAVE_STRFTIME)
	check_include_files(strings.h HAVE_STRINGS_H)
	check_include_files(string.h HAVE_STRING_H)
	check_symbol_exists(DIR sys/dir.h HAVE_SYS_DIR_H)
	check_include_files(sys/mman.h HAVE_SYS_MMAN_H)
	check_symbol_exists(DIR sys/ndir.h HAVE_SYS_NDIR_H)
	check_include_files(sys/select.h HAVE_SYS_SELECT_H)
	check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
	check_include_files(sys/stat.h HAVE_SYS_STAT_H)
	check_include_files(sys/timeb.h HAVE_SYS_TIMEB_H)
	check_include_files(sys/time.h HAVE_SYS_TIME_H)
	check_include_files(sys/types.h HAVE_SYS_TYPES_H)
	check_function_exists(time HAVE_TIME)
	check_include_files(time.h HAVE_TIME_H)
	check_include_files(unistd.h HAVE_UNISTD_H)
	check_function_exists(va_copy HAVE_VA_COPY)
	check_function_exists(vfprintf HAVE_VFPRINTF)
	check_function_exists(vsnprintf HAVE_VSNPRINTF)
	check_function_exists(vsprintf HAVE_VSPRINTF)
	check_function_exists(__va_copy HAVE___VA_COPY)
	check_c_source_compiles("
		#include <stdlib.h>
		#include <iconv.h>
		extern
		#ifdef __cplusplus
		\"C\"
		#endif
		#if defined(__STDC__) || defined(__cplusplus)
		size_t iconv(iconv_t cd, char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft);
		#else
		size_t iconv();
		#endif
		int main() { return 0; }
	" ICONV_CONST_TEST)
	if(NOT ICONV_CONST_TEST)
		set(ICONV_CONST "const")
	endif()
	set(LT_OBJDIR ".libs/")
	check_c_source_compiles("
		#include <sys/socket.h>
		#include <sys/types.h>
		int main() { (void) send(1, (const char*) \"\", 1, 1); return 0; }
	" SEND_ARG2_CAST_CONST)
	if(NOT SEND_ARG2_CAST_CONST)
		set(SEND_ARG2_CAST "(char *)")
	else()
		set(SEND_ARG2_CAST "/**/")
	endif()
	check_include_files("float.h;stdarg.h;stdlib.h;string.h" STDC_HEADERS)
	check_c_source_compiles("
		#include <stdarg.h>
		void a(va_list* ap) {};
		int main() { va_list ap1, ap2; a(&ap1); ap2 = (va_list) ap1; return 0; }
	" VA_LIST_IS_ARRAY_TEST)
	if(VA_LIST_IS_ARRAY_TEST)
		set(VA_LIST_IS_ARRAY FALSE)
	else()
		set(VA_LIST_IS_ARRAY TRUE)
	endif()
	check_c_source_compiles("
		#include <stddef.h>
		#include <sys/socket.h>
		#include <sys/types.h>
		int main() { (void) getsockopt(1, 1, 1, NULL, (socklen_t*) NULL); return 0; }
	" XML_SOCKLEN_T_SOCKLEN_T)
	if(XML_SOCKLEN_T_SOCKLEN_T)
		set(XML_SOCKLEN_T socklen_t)
	else()
		check_c_source_compiles("
			#include <stddef.h>
			#include <sys/socket.h>
			#include <sys/types.h>
			int main() { (void) getsockopt(1, 1, 1, NULL, (size_t*) NULL); return 0; }
		" XML_SOCKLEN_T_SIZE_T)
		if(XML_SOCKLEN_T_SIZE_T)
			set(XML_SOCKLEN_T size_t)
		else()
			check_c_source_compiles("
				#include <stddef.h>
				#include <sys/socket.h>
				#include <sys/types.h>
				int main() { (void) getsockopt (1, 1, 1, NULL, (int*) NULL); return 0; }
			" XML_SOCKLEN_T_INT)
			set(XML_SOCKLEN_T int)
		endif()
	endif()
	configure_file(${SRC_DIR}/config.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
endif()
configure_file(${SRC_DIR}/include/libxml/xmlversion.h.in libxml/xmlversion.h)

set(LIBXML2_HDRS
  ${SRC_DIR}/include/libxml/c14n.h
  ${SRC_DIR}/include/libxml/catalog.h
  ${SRC_DIR}/include/libxml/chvalid.h
  ${SRC_DIR}/include/libxml/debugXML.h
  ${SRC_DIR}/include/libxml/dict.h
  ${SRC_DIR}/include/libxml/DOCBparser.h
  ${SRC_DIR}/include/libxml/encoding.h
  ${SRC_DIR}/include/libxml/entities.h
  ${SRC_DIR}/include/libxml/globals.h
  ${SRC_DIR}/include/libxml/hash.h
  ${SRC_DIR}/include/libxml/HTMLparser.h
  ${SRC_DIR}/include/libxml/HTMLtree.h
  ${SRC_DIR}/include/libxml/list.h
  ${SRC_DIR}/include/libxml/nanoftp.h
  ${SRC_DIR}/include/libxml/nanohttp.h
  ${SRC_DIR}/include/libxml/parser.h
  ${SRC_DIR}/include/libxml/parserInternals.h
  ${SRC_DIR}/include/libxml/pattern.h
  ${SRC_DIR}/include/libxml/relaxng.h
  ${SRC_DIR}/include/libxml/SAX.h
  ${SRC_DIR}/include/libxml/SAX2.h
  ${SRC_DIR}/include/libxml/schemasInternals.h
  ${SRC_DIR}/include/libxml/schematron.h
  ${SRC_DIR}/include/libxml/threads.h
  ${SRC_DIR}/include/libxml/tree.h
  ${SRC_DIR}/include/libxml/uri.h
  ${SRC_DIR}/include/libxml/valid.h
  ${SRC_DIR}/include/libxml/xinclude.h
  ${SRC_DIR}/include/libxml/xlink.h
  ${SRC_DIR}/include/libxml/xmlIO.h
  ${SRC_DIR}/include/libxml/xmlautomata.h
  ${SRC_DIR}/include/libxml/xmlerror.h
  ${SRC_DIR}/include/libxml/xmlexports.h
  ${SRC_DIR}/include/libxml/xmlmemory.h
  ${SRC_DIR}/include/libxml/xmlmodule.h
  ${SRC_DIR}/include/libxml/xmlreader.h
  ${SRC_DIR}/include/libxml/xmlregexp.h
  ${SRC_DIR}/include/libxml/xmlsave.h
  ${SRC_DIR}/include/libxml/xmlschemas.h
  ${SRC_DIR}/include/libxml/xmlschemastypes.h
  ${SRC_DIR}/include/libxml/xmlstring.h
  ${SRC_DIR}/include/libxml/xmlunicode.h
  ${SRC_DIR}/include/libxml/xmlwriter.h
  ${SRC_DIR}/include/libxml/xpath.h
  ${SRC_DIR}/include/libxml/xpathInternals.h
  ${SRC_DIR}/include/libxml/xpointer.h
)

set(LIBXML2_SRCS
  ${SRC_DIR}/buf.c
  ${SRC_DIR}/c14n.c
  ${SRC_DIR}/catalog.c
  ${SRC_DIR}/chvalid.c
  ${SRC_DIR}/debugXML.c
  ${SRC_DIR}/dict.c
  ${SRC_DIR}/encoding.c
  ${SRC_DIR}/entities.c
  ${SRC_DIR}/error.c
  ${SRC_DIR}/globals.c
  ${SRC_DIR}/hash.c
  ${SRC_DIR}/HTMLparser.c
  ${SRC_DIR}/HTMLtree.c
  ${SRC_DIR}/legacy.c
  ${SRC_DIR}/list.c
  ${SRC_DIR}/nanoftp.c
  ${SRC_DIR}/nanohttp.c
  ${SRC_DIR}/parser.c
  ${SRC_DIR}/parserInternals.c
  ${SRC_DIR}/pattern.c
  ${SRC_DIR}/relaxng.c
  ${SRC_DIR}/SAX.c
  ${SRC_DIR}/SAX2.c
  ${SRC_DIR}/schematron.c
  ${SRC_DIR}/threads.c
  ${SRC_DIR}/tree.c
  ${SRC_DIR}/uri.c
  ${SRC_DIR}/valid.c
  ${SRC_DIR}/xinclude.c
  ${SRC_DIR}/xlink.c
  ${SRC_DIR}/xmlIO.c
  ${SRC_DIR}/xmlmemory.c
  ${SRC_DIR}/xmlmodule.c
  ${SRC_DIR}/xmlreader.c
  ${SRC_DIR}/xmlregexp.c
  ${SRC_DIR}/xmlsave.c
  ${SRC_DIR}/xmlschemas.c
  ${SRC_DIR}/xmlschemastypes.c
  ${SRC_DIR}/xmlstring.c
  ${SRC_DIR}/xmlunicode.c
  ${SRC_DIR}/xmlwriter.c
  ${SRC_DIR}/xpath.c
  ${SRC_DIR}/xpointer.c
  ${SRC_DIR}/xzlib.c
)

add_library(LibXml2 STATIC ${LIBXML2_HDRS} ${LIBXML2_SRCS})
target_include_directories(LibXml2 PUBLIC "${SRC_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}")
