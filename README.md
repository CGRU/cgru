##CGRU
It is a Computer Graphics Tools Pack.
Main its tools is Afanasy and Rules.
Other tools are satellites for that ones,
	like dailies encoding scripts and software submission plug-ins.

**Afanasy** is a render farm manager.

**Rules** is a Web based CG projects tracker.

[Documentation](http://cgru.info)

[Forum](http://forum.cgru.info)

You can ask any questions on a CGRU forum.

GitHub issues are designed for code developing.

###Coding rules:

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
- Do not use "!" as NOT, it is not noticeable while code passing view, much more easy to notice `false == `.
- Use `false == var` and `NULL == val` and not `var == false` or `var == NULL`.
As if you miss one '=' character, it will not be an error, it will be assignment, not comparison.
- Use `const &` to pass complex types as function parameters to not to copy class instance.

####Example:

	// Function with long parameters list:
	bool someFunction(
			std::string & o_status,
			const std::string & i_param1,
			const std::string & i_param2,
			const std::string & i_param3,
			const std::string & i_param4
		)
	{
		...

		if( false == variable_name )
		{
			o_status = "error";
			return false;
		}

		return true;
	}

You will notice, that some parts of the code is not following these rules. This
is mostly because such code has been written before these rules were created or
changed.


### Some names:
**CGRU** - came from CG - Rules. It has two meanings: computer graphics principles and computer graphics is a cool thing.

**Afanasy** - came from a Greek name meaning [immortal](http://en.wikipedia.org/wiki/Afanasy). 

**Rules** - simple came from rules! This project tracker is based on defined rules.
Mostly project structure rules, where to store sources, references, dailies, outputs.
As Rules has not its own database, it walks file-server folders structure.

