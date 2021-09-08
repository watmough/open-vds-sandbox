set -e -u
base_dir=$(realpath $(dirname $BASH_SOURCE))

openvds_path=$(realpath "$base_dir/../..")
cmake_args="-DBUILD_TESTS=OFF"
openvds_version=""
name="openvds"

cmake_generator=""
platform_name=""
skplat_name=""
distribution=""
output_dir=""
auditwheels="no"
libdir_suffix=""
[[ -d /usr/lib64 ]] && libdir_suffix="64"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  cmake_generator="Ninja"
  platform_name="linux"
  skplat_name="linux-x86_64"
  toolset=""
elif [[ "$OSTYPE" == "darwin"* ]]; then
  cmake_generator="Ninja"
  platform_name="mac"
  skplat_name="maxosx-10.6-x86_64"
  toolset=""
elif [[ "$OSTYPE" == "msys" ]]; then
  cmake_generator="Visual Studio 16 2019"
  platform_name="win"
  skplat_name="win-amd64"
  toolset="-Tv140"
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
[[ -d $openvds_path/dist ]] && rm -rf $openvds_path/dist
[[ -d $openvds_path/_skbuild ]] && rm -rf $openvds_path/_skbuild/linux*
[[ -d $openvds_path/_skbuild ]] && rm -rf $openvds_path/_skbuild/win*

mkdir -p $openvds_path/binpackage/$name-$openvds_version
mkdir -p $openvds_path/binpackage/python/$distribution/

for python_executable in "${python_executables[@]}"; do
  python_ver=$("$python_executable" -c "import sys;  print('.'.join(map(str, sys.version_info[:2])))")
  python_root_dir=$("$python_executable" -c "import sys; import os; print(os.path.dirname(sys.executable))")
  
  openvds_path=$(realpath $openvds_path)
  
  skbuild_platform="$skplat_name-$python_ver"
  
  skbuild_dir="$openvds_path/_skbuild/$skbuild_platform"
  build_dir="$openvds_path/_skbuild/internal_build_dir"
  [[ -d $skbuild_dir ]] || mkdir -p $skbuild_dir
  [[ -d $build_dir ]] || mkdir -p $build_dir
  skbuild_dir=$(realpath $skbuild_dir)
  build_dir=$(realpath $build_dir)
  
  cd "$build_dir"
 
  echo "Do $python_executable to $skbuild_dir"
  if [[ "$platform_name" == "win" ]]; then
    cmake -DPython3_ROOT_DIR="$python_root_dir" -DCMAKE_INSTALL_PREFIX=$skbuild_dir/cmake-install $cmake_args -G"$cmake_generator" $toolset $openvds_path
    cmake --build . --config Debug --target install
    cmake --build . --config Release --target install
  else
    cmake -DPython3_ROOT_DIR="$python_root_dir" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$skbuild_dir/cmake-install $cmake_args -G"$cmake_generator" $toolset $openvds_path
    cmake --build . --config Release --target install
  fi
 
  cd "$openvds_path"
  [[ -d "$skbuild_dir/cmake-build" ]] || mkdir -p "$skbuild_dir/cmake-build"
  cp -r "$build_dir"/* "$skbuild_dir/cmake-build"
  
  "$python_executable" setup.py --skip-cmake bdist_wheel

  cp -r $skbuild_dir/cmake-install/* binpackage/$name-$openvds_version

  if [[ "$auditwheels" == "yes" ]]; then
    old_dir=$PWD
    cd $openvds_path/dist
    LD_LIBRARY_PATH=$skbuild_dir/cmake-install/lib${libdir_suffix} auditwheel repair *.whl
    cp wheelhouse/*manylinux* $openvds_path/binpackage/$name-$openvds_version/
    mv wheelhouse/*manylinux* $openvds_path/binpackage/python/$distribution/
    cd $old_dir
  else
    cp $openvds_path/dist/* $openvds_path/binpackage/$name-$openvds_version/
    mv $openvds_path/dist/* $openvds_path/binpackage/python/$distribution/
  fi
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
