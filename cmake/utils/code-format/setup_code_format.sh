#!/bin/bash

# Helper script to setup formatting stuff in the cloned repository
# - setup git pre-commit hook
# - verify/setup configuration files
# - verify formatting tools are installed

TOP_DIR=$(git rev-parse --show-toplevel)
CODE_FORMAT_DIR=cmake/utils/code-format
PROJECTS_DIR=sources

pushd ${TOP_DIR}
# setup git hook
ln -i -s ../../${CODE_FORMAT_DIR}/git_hooks_pre-commit .git/hooks/pre-commit
if [ ! -x .git/hooks/pre-commit ] ; then
	chmod +x .git/hooks/pre-commit
fi

# verify/setup config files
if [ ! -e .clang-format ] ; then
	read -p "No '.clang-format' found in the top repo dir, would you like to create a symlink to './${CODE_FORMAT_DIR}/_clang-format' (Y/n)?" y
	y=$(echo $y | tr '[:upper:]' '[:lower:]')
	if [ x"$y" = x"y" -o x"$y" = x ] ; then
		ln -i -s ./${CODE_FORMAT_DIR}/_clang-format ./.clang-format
	fi
fi
if [ ! -e .cmake-format ] ; then
	read -p "No '.cmake-format' found in the top repo dir, would you like to create a symlink to './${CODE_FORMAT_DIR}/_cmake-format' (Y/n)?" y
	y=$(echo $y | tr '[:upper:]' '[:lower:]')
	if [ x"$y" = x"y" -o x"$y" = x ] ; then
		ln -i -s ./${CODE_FORMAT_DIR}/_cmake-format ./.cmake-format
	fi
fi

# verify/setup customized clang formatting per projects
for f in $(ls ./${CODE_FORMAT_DIR}/_clang-format_*); do 
	project=${f#*clang-format_}
	if [ ! -e ${PROJECTS_DIR}/${project}.clang-format ] ; then
		read -p "No '${PROJECTS_DIR}/${project}/.clang-format' found, would you like to create a symlink to './${CODE_FORMAT_DIR}/_clang-format_${project}' (Y/n)?" y
		y=$(echo $y | tr '[:upper:]' '[:lower:]')
		if [ x"$y" = x"y" -o x"$y" = x ] ; then
			pushd ${PROJECTS_DIR}/${project} > /dev/null
			ln -i -s ../../${CODE_FORMAT_DIR}/_clang-format_${project} ./.clang-format
			popd > /dev/null
		fi
	fi
done

# show info
echo
ls -g .git/hooks/pre-commit
ls -g .clang-format
ls -g .cmake-format
popd

# verify clang-format is installed
echo -e "\nclang-format version:"
if ! clang-format -version ;  then
	read -p "Would you like to install clang-format with command 'sudo apt-get install clang-format' (Y/n)?" y
	y=$(echo $y | tr '[:upper:]' '[:lower:]')
	if [ x"$y" = x"y" -o x"$y" = x ] ; then
		sudo apt-get install clang-format
	fi
fi

# verify cmake-format is installed
echo -e "\ncmake-format version:"
if ! cmake-format --version; then
	read -p "Would you like to install cmake-format with command 'sudo pip install cmake-format' (Y/n)?" y
	y=$(echo $y | tr '[:upper:]' '[:lower:]')
	if [ x"$y" = x"y" -o x"$y" = x ]; then
		sudo pip install cmake-format
	fi
fi

