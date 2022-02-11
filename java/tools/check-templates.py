import glob
import sys

ILLEGAL_PATTERNS = [
    'Hue::',
    'int dimensionsND',
    'int interpolationMode',
    'int projectedDimension',
    'com_hue_proxylib',
    'proxyinterface',
    'ProxyInterface',
]

cpp_files = glob.glob('javagen_templates/*.cpp')
java_files = glob.glob('javagen_templates/*.java')
templates = list(cpp_files)
templates.extend(java_files)
for t in templates:
    with open(t, 'r') as file:
        contents = file.readlines()
        for lineno in range(0, len(contents)):
            line = contents[lineno]
            for p in ILLEGAL_PATTERNS:
                if p in line:
                    print(f'Illegal pattern {p} found in {t}, line {lineno + 1}', file=sys.stderr)
                