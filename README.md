# Array member viewer - Visual Studio extension
VS extension for viewing a member from all entries in an array of structs.
For example, if you have 

```
struct S { int a,b,c,d,e; };
S s[5];
```

Then you can write `ArrayMember("s",".d",10)` in the watch window and get something like this:

```
-		ArrayMember("s",".d",10)	...	ArrayMemberDesc
		s[0].d	0	int
		s[1].d	0	int
		s[2].d	0	int
		s[3].d	0	int
		s[4].d	1	int
```

To use it,
1) Install ./Release/ArrayMemberVisualizer.vsix
2) Add this code somewhere in the project you're debugging:

```
struct ArrayMemberDesc
{
    const hkUint64 base;
    const hkUint64 member;
    int count;
};

ArrayMemberDesc ArrayMember(const char* base, const char* member, int count)
{
    return ArrayMemberDesc{hkUint64(base), hkUint64(member), count};
}
```

To compile it, you need the Visual Studio SDK, https://docs.microsoft.com/en-us/visualstudio/extensibility/visual-studio-sdk?view=vs-2019.  This project is derived from https://github.com/microsoft/ConcordExtensibilitySamples/tree/master/CppCustomVisualizer
