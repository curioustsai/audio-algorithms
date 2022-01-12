# Clang-format / cmake-format support

## To install:
Formatting verification git-hook script should be installed each time
the repository is cloned.
To install:
1. Either run setup script:
	`./cmake/utils/code-format/setup_code_format.sh`
2. Or do it manually from git root directory:

- install clang-format utility, f.e. on Ubuntu:
	`apt install clang-format`

- install cmake-format utility, f.e. on Ubuntu:
	`pip install cmake_format`

- create symlink `.git/hooks/pre-commit` to `git_hooks_pre-commit` script:
	`ln -i -s ../../cmake/utils/code-format/git_hooks_pre-commit .git/hooks/pre-commit`

- make sure that `pre-commit` git hook script has executable flag enabled:
	`if [ ! -x .git/hooks/pre-commit ] ; then echo "run: chmod +x .git/hooks/pre-commit"; fi`

- create symlinks to format configuration files in the top directory with the sources:
	`ln -s ./cmake/utils/code-format/.clang-format ./`
	`ln -s ./cmake/utils/code-format/.clang-format ./`

## Plugins to IDEs
Also it would be very useful to install clang-format plugin to your text editor.
There are plugins available for vim, VS code, Eclipse and other well-know IDE and editors.
Configure pluging to apply formatting on writing file.

There are only few editors that have plugins for cmake-format.

## Manual formatting
To manually format sources files the following command can be used:
	`clang-format -style=file -i ./<path_to_file> ./<path_to_file2> ...`

Similarly, cmake files can be formatted manually using command:
	`cmake-format -i ./<path_to_cmake_file> ./<path_to_cmake_file2> ...`

