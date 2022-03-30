import glob
import sys

ILLEGAL_PATTERNS = [
    'proxyinterface',
    'ProxyInterface',
]

REPLACE_PATTERNS = {
    'VolumeDataChannelDescriptor.Format': 'VolumeDataFormat',
#    'VolumeDataFormat': 'VolumeDataChannelDescriptor::Format',
#    'VCVoxelFormat': 'VolumeDataChannelDescriptor.Format',
    'Hue::Util': 'OpenVDS',
    'Hue::HueSpaceLib': 'OpenVDS',
    'int dimensionsND': 'long dimensionsND',
    'int interpolationMethod': 'long interpolationMethod',
    'int projectedDimensions':  'long projectedDimensions',
    'jobject jproxyinterface, ': '',
    ', jproxyinterface': '',
    'ProxyInterface.getInstanceSafe(), ': '',
    'ProxyInterface proxyInterface, ': '',
    'com_hue_proxylib': 'org_opengroup_openvds',
    'com.hue.proxylib': 'org.opengroup.openvds',
}

def main():
    force_update = '-f' in sys.argv[1:]
    auto_update = '-u' in sys.argv[1:] or force_update
    cpp_files = glob.glob('javagen_templates/*.cpp')
    java_files = glob.glob('javagen_templates/*.java')
    templates = cpp_files + java_files
    all_patterns = ILLEGAL_PATTERNS + list(REPLACE_PATTERNS.keys())
    any_errors = False
    for t in templates:
        rewrite = False
        template_ok = True
        lines = []
        with open(t, 'r') as file:
            lines = file.readlines()
            for lineno in range(0, len(lines)):
                line = lines[lineno]
                if auto_update:
                    for p in REPLACE_PATTERNS:
                        if p in line:
                            print(f'Replacing pattern "{p}" in {t}, line {lineno + 1}', file=sys.stdout)
                            lines[lineno] = line.replace(p, REPLACE_PATTERNS[p])
                            rewrite = True
                line = lines[lineno]
                for p in all_patterns:
                    if p in line:
                        print(f'Illegal pattern "{p}" found in {t}, line {lineno + 1}', file=sys.stderr)
                        template_ok = False
        if rewrite:
            if template_ok or force_update:
                with open(t, 'w') as outfile:
                    outfile.writelines(lines)
            else:
                print('Automatic update did not fix all errors in {t}. Invoke this script with -f to force update anyway.', sys.stdout)
        any_errors = any_errors or not template_ok
    if any_errors:
        print('Invoke this script with -u to try to automatically update templates.', file=sys.stdout)
    
if __name__ == "__main__":
    main()