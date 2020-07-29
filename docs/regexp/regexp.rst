Regular Expressions
===================

This is a standard Regular Expressions.
They are realized in almost all languages like C, C++, Python, JavaScript, bash, Perl, PHP, Ruby and others.
Some times such expressions called "Perl-like" as they were popularized within Perl at first.

AFANASY uses this expressions in hosts mask, depend masks and other patterns.

+---------------------+---------------------------+---------------------------------------------------------------------------+
| Examples Expression | Some Matched Names        | Description                                                               |
+=====================+===========================+===========================================================================+
| r1|r7|r12           | r1 r7 r12                 | Only specified machines ("|" - means "or")                                |
+---------------------+---------------------------+---------------------------------------------------------------------------+
| r1.                 | r11-r19 r1a-r1z           | Names starts with "r" plus one any character ("." - any (one) character)  |
+---------------------+---------------------------+---------------------------------------------------------------------------+
| r.*                 | r0 r1 rN rnd roman ...    | All computers which name starts with "r" ("*" - any number or characters) |
+---------------------+---------------------------+---------------------------------------------------------------------------+
| r1.*                | r1 r10 r11 r102 r1asd ... | All computers which name starts with "r1"                                 |
+---------------------+---------------------------+---------------------------------------------------------------------------+
| r1[178]             | r11 r17 r18               | Only specified machines, [1 or 7 or 8]                                    |
+---------------------+---------------------------+---------------------------------------------------------------------------+
| r1[1-5]             | r11 r12 r13 r14 r15       | Only specified machines, [from 1 to 5]                                    |
+---------------------+---------------------------+---------------------------------------------------------------------------+
| r0.|r1[1-5]         | r00-r09 r11-r15           | All r0# and from r11 to r15                                               |
+---------------------+---------------------------+---------------------------------------------------------------------------+

Detailed documentation, examples, constructors and testers:

 - http://en.wikipedia.org/wiki/Regular_expression
 - http://www.google.com/search?q=Regular+Expressions
 - http://www.google.com/search?q=Regular+Expressions+Examples
 - http://www.google.com/search?q=Regular+Expressions+Tester


RegExp Checker
--------------

A simple dialog to check regular expressions provided with CGRU.

You can launch it from keeper: *Afanasy -> Check RegExp...*

.. image:: regexp_checker.png

Any render01-render04 or workstation01-workstation04.

