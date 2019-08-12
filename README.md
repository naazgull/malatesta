Usage
-----

    malatesta -w <local dir>,<full qualified remote dir> -x <exclude dirs regex> -f <included files regex>

Options
-------

- **-w** local and remote uri pair - multiple occurrences accepted
- **-x** directory pattern to be excluded from watches - multiple occurrences accepted
- **-f** file pattern to include in watches - multiple occurrences accepted

Example
-------

    malatesta -w /home/ss/dev,scott_summers@mydev.com:/home/ss/dev -w /home/ss/bin,scott_summers@mydev.com:/home/ss/bin -x "\\.(.*)" -f "([^.#])(.*)(\\.cc|\\.cpp|\\.h|\\.hpp|\\.txt)"
