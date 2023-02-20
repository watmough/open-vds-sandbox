set -e -u
base_dir=$(realpath $(dirname $BASH_SOURCE))

openvds_path=$(realpath "$base_dir/../..")
cmake_args=""
openvds_version=""
name="openvds"

platform_name=""
skplat_name=""
distribution=""
output_dir=""
auditwheels="no"
libdir_suffix=""
[[ -d /usr/lib64 ]] && libdir_suffix="64"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  platform_name="linux"
  skplat_name="linux-x86_64"
elif [[ "$OSTYPE" == "darwin"* ]]; then
  platform_name="mac"
  skplat_name="maxosx-10.6-x86_64"
elif [[ "$OSTYPE" == "msys" ]]; then
  platform_name="win"
  skplat_name="win-amd64"
fi
  
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -p|--python)
    python_executables+=("$2")
    shift # past argument
    shift # past value
    ;;
    -v|--version)
    openvds_version="$2"
    shift # past argument
    shift # past value
    ;;
    -c|--cmake_arg)
    cmake_args="$cmake_args $2"
    shift # past argument
    shift # past value
    ;;
    -d|--distribution)
    distribution="$2"
    shift # past argument
    shift # past value
    ;;
    -n|--name)
    name="$2"
    shift # past argument
    shift # past value
    ;;
    -o|--output_dir)
    output_dir="$2"
    shift # past argument
    shift # past value
    ;;
  -a|--auditwheels)
    auditwheels="yes"
    shift
    ;;
    *)    # unknown option
    openvds_path="$1" # save it in an array for later
    shift # past argument
    ;;
esac
done

if which javac ; then
  java_home=$(which java)
  java_home=$(dirname "$java_home")
  java_home=$(realpath "$java_home/..")
  if [[ "$platform_name" == "win" ]]; then
    java_home=$(cygpath -m "$java_home")
  fi

  java_cmake_arg="-DJAVA_HOME=$java_home"
  echo "CMAKE_ARG $java_cmake_arg"
else
  java_cmake_arg=""
fi


if [ -n "${VIRTUAL_ENV:-}" ] ; then
  BUILD_PACKAGE_OLD_VIRTUAL_ENV="${VIRTUAL_ENV:-}"

  if [ -n "${_OLD_VIRTUAL_PATH:-}" ] ; then
    BUILD_PACKAGE_OLD_VIRTUAL_PATH="${_OLD_VIRTUAL_PATH:-}"
    unset _OLD_VIRTUAL_PATH
  fi
  if [ -n "${_OLD_VIRTUAL_PYTHONHOME:-}" ] ; then
    BUILD_PACKAGE_OLD_VIRTUAL_PYTHONHOME="${_OLD_VIRTUAL_PYTHONHOME:-}"
    unset _OLD_VIRTUAL_PYTHONHOME
  fi

  if [ -n "${_OLD_VIRTUAL_PS1:-}" ] ; then
    BUILD_PACKAGE_OLD_VIRTUAL_PS1="${_OLD_VIRTUAL_PS1:-}"
    unset _OLD_VIRTUAL_PS1
  fi
fi

if [[ "$openvds_version" == "" ]]; then
  openvds_version=${openvds_version:-$(<$openvds_path/VERSION)}
fi
if [[ "$openvds_version" == "" ]]; then
  echo "Please specify OpenVDS version using -v --version"
  exit 1
fi
if [[ "${python_executables[@]}" == "" ]];  then
  echo "Please specify at least one python executable"
  exit 1
fi
if [[ "$distribution" == "" ]]; then
    distribution="$platform_name"
fi

[[ -d $openvds_path/binpackage ]] && rm -rf $openvds_path/binpackage
[[ -d $openvds_path/binpackage_test_results ]] && rm -rf $openvds_path/binpackage_test_results
[[ -d $openvds_path/dist ]] && rm -rf $openvds_path/dist
[[ -d $openvds_path/_skbuild ]] && rm -rf $openvds_path/_skbuild/linux*
[[ -d $openvds_path/_skbuild ]] && rm -rf $openvds_path/_skbuild/win*

mkdir -p $openvds_path/binpackage/$name-$openvds_version
mkdir -p $openvds_path/binpackage/python/$distribution/
mkdir -p $openvds_path/_skbuild

if [[ "$platform_name" == "win" ]]; then
  cmake_version=3.23.2
  cmake_name="cmake-$cmake_version-windows-x86_64"
  cd "$openvds_path/_skbuild"
  if [[ ! -d $cmake_name ]]; then
    curl -O -L "https://github.com/Kitware/CMake/releases/download/v$cmake_version/$cmake_name.zip"
    unzip $cmake_name.zip
  fi
  if [[ ! -d ninja ]]; then
    curl -O -L https://github.com/ninja-build/ninja/releases/download/v1.11.0/ninja-win.zip
    unzip ninja-win.zip -d ninja
  fi
  export PATH="$openvds_path/_skbuild/$cmake_name/bin:$openvds_path/_skbuild/ninja:$PATH"
  cmake_executable="$openvds_path/_skbuild/$cmake_name/bin/cmake.exe"
  ctest_executable="$openvds_path/_skbuild/$cmake_name/bin/ctest.exe"
  ninja_executable="$openvds_path/_skbuild/ninja/ninja.exe"
  cd "$openvds_path"
else
  cmake_executable=$(which cmake)
  ctest_executable=$(which ctest)
  ninja_executable=$(which ninja)
fi

for python_executable in "${python_executables[@]}"; do
  python_ver=$("$python_executable" -c "import sys;  print('.'.join(map(str, sys.version_info[:2])))")

  openvds_path=$(realpath $openvds_path)

  skbuild_platform="$skplat_name-$python_ver"

  skbuild_dir="$openvds_path/_skbuild/$skbuild_platform"
  [[ -d $skbuild_dir ]] || mkdir -p $skbuild_dir
  skbuild_dir=$(realpath $skbuild_dir)

  venv_dir="$openvds_path/_skbuild/venv_$python_ver"
  $python_executable -m venv $venv_dir
  if [[ "$platform_name" == "win" ]]; then
    source $venv_dir/Scripts/activate
  else
    source $venv_dir/bin/activate
  fi
  python_executable=$(python -c "import sys; import os; print(sys.executable.replace(os.sep, '/'))")
  $python_executable -m pip install -r $openvds_path/python/requirements-dev.txt
  python_root_dir=$("$python_executable" -c "import sys; import os; print(os.path.dirname(sys.executable))")

  deactivate nondestructive

  echo "Do $python_executable to $skbuild_dir"
  cd "$openvds_path"

  "$cmake_executable" -DPython3_ROOT_DIR="$python_root_dir" "$java_cmake_arg" -DENABLE_MSVC_TOOLSET_DIR=OFF -DCMAKE_INSTALL_PREFIX=$skbuild_dir/cmake-install $cmake_args --preset Release
  "$ninja_executable" -C out/build/Release install
  "$ctest_executable" -V --test-dir out/build/Release
  [[ -d "binpackage_test_results/$python_ver" ]] || mkdir -p "binpackage_test_results/$python_ver"
  cp -r "out/build/Release/test_results" "binpackage_test_results/$python_ver"
  cp -r "out/build/Release/test_results_java" "binpackage_test_results/$python_ver"

  [[ -d "$skbuild_dir/cmake-build" ]] || mkdir -p "$skbuild_dir/cmake-build"
  cp -r out/build/Release/* "$skbuild_dir/cmake-build"

  "$python_executable" setup.py --skip-cmake bdist_wheel

  if [[ "$auditwheels" == "yes" ]]; then
    old_dir=$PWD
    cd $openvds_path/dist
    LD_LIBRARY_PATH=$skbuild_dir/cmake-install/lib${libdir_suffix} auditwheel repair *.whl
    manylinux_wheels=( $PWD/wheelhouse/*manylinux*.whl )
    the_wheel=${manylinux_wheels[0]}
    rm -rf tmp
    mkdir tmp
    cd tmp
    unzip $the_wheel
    data_dirs=( $PWD/*.data )
    the_datadir=${data_dirs[0]}
    cd $the_datadir/data
    mkdir lib_new
    cp ../../openvds.libs/* lib_new/
    cd lib_new
    the_openvds_lib_pattern=( libopenvds* )
    the_openvds_lib=${the_openvds_lib_pattern[0]}
    for ovds_link in ../lib${libdir_suffix}/libopenvds.so*; do
      ln -s $the_openvds_lib $(basename $ovds_link)
    done
    cp -av ../lib${libdir_suffix}/libopenvds-java* .
    cp -av ../lib${libdir_suffix}/libsegy* .
    patchelf --set-rpath '$ORIGIN' *
    cd ..
    rm -rf lib${libdir_suffix}
    mv lib_new lib${libdir_suffix}
    cd $openvds_path/dist
    rm $the_wheel
    $base_dir/repair_wheel_extra tmp $the_wheel

    cp wheelhouse/*manylinux* $openvds_path/binpackage/$name-$openvds_version/
    mv wheelhouse/*manylinux* $openvds_path/binpackage/python/$distribution/
    cd $old_dir
  else
    cp $openvds_path/dist/* $openvds_path/binpackage/$name-$openvds_version/
    mv $openvds_path/dist/* $openvds_path/binpackage/python/$distribution/
  fi

  if [[ "$platform_name" == "win" ]]; then
    cd "$openvds_path"
    rm -rf $skbuild_dir/cmake-install
    "$cmake_executable" -DPython3_ROOT_DIR="$python_root_dir" "$java_cmake_arg" -DENABLE_MSVC_TOOLSET_DIR=ON -DCMAKE_INSTALL_PREFIX=$skbuild_dir/cmake-install $cmake_args --preset Release
    "$ninja_executable" -C out/build/Release install
    "$cmake_executable" -DPython3_ROOT_DIR="$python_root_dir" "$java_cmake_arg" -DENABLE_MSVC_TOOLSET_DIR=ON -DCMAKE_INSTALL_PREFIX=$skbuild_dir/cmake-install $cmake_args --preset Debug
    "$ninja_executable" -C out/build/Debug install
  fi
  cd "$openvds_path"
  cp -r $skbuild_dir/cmake-install/* binpackage/$name-$openvds_version

  rm -rf $openvds_path/dist
done


if [[ "$auditwheels" == "yes" ]]; then
  cd $openvds_path/binpackage/$name-$openvds_version
  mkdir lib${libdir_suffix}_new
  mkdir temp
  cd temp
  manylinux_wheels=( $openvds_path/binpackage/$name-$openvds_version/*manylinux*.whl )
  unzip ${manylinux_wheels[0]}
  cd ..
  cp temp/openvds.libs/* lib${libdir_suffix}_new
  cd lib${libdir_suffix}_new
  the_openvds_lib_pattern=( libopenvds* )
  the_openvds_lib=${the_openvds_lib_pattern[0]}
  for ovds_link in ../lib${libdir_suffix}/libopenvds.so*; do
    ln -s $the_openvds_lib $(basename $ovds_link)
  done
  cp -av ../lib${libdir_suffix}/libopenvds-java* .
  cp -av ../lib${libdir_suffix}/libsegy* .
  patchelf --set-rpath '$ORIGIN' *
  cd ..
  rm -rf lib${libdir_suffix}
  mv lib${libdir_suffix}_new lib${libdir_suffix}
  rm -rf temp
fi

cd $openvds_path/binpackage

if [[ "$platform_name" == "win" ]]; then
  zip -r $name-$openvds_version-$distribution.zip $name-$openvds_version
else
  tar -zcvf $name-$openvds_version-$distribution.tar.gz $name-$openvds_version
fi
rm -rf $name-$openvds_version

if [[ "$output_dir" != "" ]]; then
  cp -r * $output_dir
fi

if [ -n "${BUILD_PACKAGE_OLD_VIRTUAL_ENV:-}" ] ; then
  VIRTUAL_ENV="${BUILD_PACKAGE_OLD_VIRTUAL_ENV:-}"
  unset BUILD_PACKAGE_OLD_VIRTUAL_ENV
  if [ -n "${BUILD_PACKAGE_OLD_VIRTUAL_PATH:-}" ] ; then
    _OLD_VIRTUAL_PATH="${BUILD_PACKAGE_OLD_VIRTUAL_PATH:-}"
    unset BUILD_PACKAGE_OLD_VIRTUAL_PATH
  fi
  if [ -n "${BUILD_PACKAGE_OLD_VIRTUAL_PYTHONHOME:-}" ] ; then
    _OLD_VIRTUAL_PYTHONHOME="${BUILD_PACKAGE_OLD_VIRTUAL_PYTHONHOME:-}"
    unset BUILD_PACKAGE_OLD_VIRTUAL_PYTHONHOME
  fi

  if [ -n "${BUILD_PACKAGE_OLD_VIRTUAL_PS1:-}" ] ; then
    _OLD_VIRTUAL_PS1="${BUILD_PACKAGE_OLD_VIRTUAL_PS1:-}"
    unset BUILD_PACKAGE_OLD_VIRTUAL_PS1
  fi
fi

