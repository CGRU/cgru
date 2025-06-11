## CGRU <img src="https://raw.githubusercontent.com/CGRU/cgru/master/favicon.ico" align="right" width="90">

CGRU is the Computer Graphics Tools Pack.

It consists of the main tools _Afanasy_ and _Rules_.
Other tools are arranged as satellites around these two,
like _dailies encoding scripts_ and _software submission plug-ins_.

**Afanasy** is a render farm manager.

**Rules** is a Web based CG projects tracker.

[Site](https://cgru.info)

[Documentation](https://cgru.readthedocs.io)

[Forum](https://forum.cgru.info)

You can ask any questions on the CGRU forum.

GitHub issues are used for code development and bug tracking.

### Coding rules:

- Indentation - TABS. You can vary tab length, default 4 spaces length is normal.
	- Python Indentation - SPACES(4). It is not a problem to tune any modern text editor for each file type.
- Code alignment - SPACES, it should not break on various tab length.
- Variable names: variable_name.
- Function names: functionName.
- Class names: ClassName.
- Use prefixes to make code more readable:
	- `i_` - Input variables.
	- `o_` - Output variables.
	- `m_` - Class members.
	- `ms_` - Static class members.
	- `g_` - External variables.
	- `v_` - Virtual functions.
- Do not use "!" as NOT, since it is not noticeable for doing code review. It is much more easy to notice `false == `.
- Use `false == var` and `NULL == val` instead of `var == false` or `var == NULL`.
As if you miss one '=' character, it will not be an error, it will be assignment, not comparison.
- Use `const &` to pass complex types as function parameters to not to copy class instance.

#### Example:

```c
// Function with long parameters list:
bool someFunction(std::string &o_status, const std::string &i_param1, const std::string &i_param2,
	const std::string &i_param3, const std::string &i_param4)
{
	...

	if (false == variable_name)
	{
		o_status = "error";
		return false;
	}
	return true;
}
```

You will notice, that some parts of the code are not following these rules. This
is mostly because such code has been written before these rules were created or
changed.

The typical file header and code style for C++ and JS files can automatically be applied with the script 
[utilities/maintenance/codeStyleCheck.php](https://github.com/CGRU/cgru/blob/master/utilities/maintenance/codeStyleCheck.php) see the file for instructions of setup and usage. 

The file header shall contain as much information as possible about the file, you can find a good example here:
[afanasy/browser/monitor.js](https://github.com/CGRU/cgru/blob/master/afanasy/browser/monitor.js)

### Code Formatting

This code uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to format both the Javascript as well as the C++ code.

#### Auto formatting

It provides a [pre-commit](https://pre-commit.com/) hook for auto-formatting. It can be installed by running:

```sh
pipx install pre-commit
pre-commit install
```

This hook will run and format the code on the next "git commmit".

#### Manual formatting

You can also manually format the code by running:

```sh
clang-format -i the-source-file.cpp the-header-file.h
```


### Some names:
**CGRU** - came from CG - Rules. It has two meanings: computer graphics principles and computer graphics is a cool thing.

**Afanasy** - came from a Greek name meaning [immortal](http://en.wikipedia.org/wiki/Afanasy). 

**Rules** - simple came from rules! This project tracker is based on defined rules.
Mostly project structure rules, where to store sources, references, dailies, outputs.
As Rules does not have its own database, it walks file-server folders structure.
