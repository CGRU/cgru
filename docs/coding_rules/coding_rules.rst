Coding Rules
============

 - Indentation: TAB. You can vary tab length, default 4 spaces length is normal. Except Python.
 - Python indentation: 4 SPACES.
 - Code alignment: SPACES, it should not break on various tab length.
 - Variable names: ``variable_name``.
 - Function names: ``functionName``.
 - Class names: ``ClassName``.
 - Use prefixes to make code more readable:

    - ``i_``: Input variables.
    - ``o_``: Output variables.
    - ``m_``: Class members.
    - ``ms_``: Static class members.
    - ``g_``: External variables.
    - ``v_``: Virtual functions.

 - Do not use ``!`` as NOT, it is not noticeable while code passing view, much more easy to notice ``false ==``
 - Use ``const &`` to pass complex types as function parameters to not to copy class instance.

**Example:**

.. code-block:: c

    void someFunction(const SomeClass & i_some_class, int i_some_type)
    {
        ...
    }

    // Function with a long parameters list:
    bool someOtherFunction(
            std::string & o_status,
            const std::string & i_param1,
            const std::string & i_param2,
            const std::string & i_param3,
            const std::string & i_param4
        )
    {
        ...

        if (false == variable_name)
        {
            o_status = "error";
            return false;
        }

        return true;
    }

.. note::

    Afanasy code is not all written by this rules.
    But better to write new code by this rules, and may be replace near parts.

.. warning::

    Old code or new code written not by rules can be re-factored any time.

