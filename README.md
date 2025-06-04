# pulib - C Framework(because I have a skill issue and do not want to maintain a single-header library)
## **pulib** consists of various modules that include:
<p>cio - console io functions, such as reading primitives, reading option out of provided, running a process, etc;</p>
<p>da - simple header that adds dynamic array functionality for structs that have fields int length, int capacity, T* items;</p>
<p>fio - file io functions, such as reading directories, writing and reading primitives from the file, dealing with path strings, etc;</p>
<p>helpers - uncategorized mess;</p>
<p>mem - ArenaAllocator and a little more;</p>
<p>str - cstr(char* that is expected to be null- terminated), StringBuilder, StringView, and all the variety of things you might want to do with them. Note: .items of non-empty StringBuilder is a cstr;</p>
