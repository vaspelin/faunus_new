#!/usr/bin/env python

import os
import sys
import fnmatch
from glob import glob

#from IPython.Shell import IPShellEmbed
#ipshell = IPShellEmbed([])

try:
    from pyplusplus.module_builder import module_builder_t
    from pygccxml.declarations.matchers import access_type_matcher_t
except:
    print "import from pyplusplus or pygccxml failed, aborting"
    sys.exit(1)


def locate(pattern, root=os.curdir):
    '''
    Locate all files matching supplied filename pattern
    in and below supplied root directory.
    '''

    for path, dirs, files in os.walk(os.path.abspath(root)):
        for filename in fnmatch.filter(files, pattern):
            yield os.path.join(path, filename)


#
# settings
#

# list of header files that will be "seen" by the wrapper generator
header_files_list = ['faunus/faunus.h',
                     'faunus/common.h',
                     'faunus/point.h',
                     'faunus/particles.h',
                     'faunus/slump.h',
                     'faunus/inputfile.h',
                     'faunus/species.h',
                     'faunus/io.h',
                     'faunus/container.h',
                     'faunus/potentials/pot_coulomb.h',
                     'faunus/moves/base.h',
                     'faunus/moves/translational.h',
                     'faunus/group.h',
                     'faunus/analysis.h',
                     'faunus/widom.h',
                     'faunus/energy.h',
                     'faunus/ensemble.h',
                     'faunus/histogram.h',
                     'faunus/xytable.h',
                     'faunus/mcloop.h',
                    ]

header_files = header_files_list
#header_files = list(locate('*.h', '../../include/faunus'))

classes = ['point',                             # point.h
           'particle',
           'spherical',
           'particles',                         # particles.h
           'random',                            # slump.h
           'randomDefault',
           'randomTwister',
           'inputfile',                         # inputfile.h
           'atoms',                             # species.h
           'container',                         # container.h
           'cell',
           'box',
           'slit',
           'clutch',
           'cylinder',
           'ensemble',                          # ensemble.h
           'canonical',
           'pot_coulomb',                       # potentials
           'pot_lj',
           'energybase',                        # energy.h
           'interaction<Faunus::pot_coulomb>',
           'interaction<Faunus::pot_hscoulomb>',
           'markovmove',                        # moves
           'saltmove',
           'group',                             # group.h
           'salt',
           'analysis',                          # analysis.h
           'systemenergy',
           'widom',                             # widom.h
           #'widomSW',
           'mcloop',                            # mcloop.h
           'ioaam',                             # io.h
           ]

other_stuff = ['faunus_splash']

# template instantiation requests
# dictionary with the fillowing structure:
# key - C++ template
# value - a tuple of of two-tuples, each of them contains template arguments and a python alias
template_data = {'Faunus::interaction': (('Faunus::pot_coulomb','interaction_coulomb'),
                                         ('Faunus::pot_hscoulomb', 'interaction_hscoulomb'),
                                        ),
                }

#
# generator code
#

# documentation says it is safer to use absolute paths for the header files
header_files_abs = [os.path.abspath(p) for p in header_files]

generated_header_template = """%s

namespace pyplusplus{ namespace aliases{

%s

    inline void instantiate() {
%s
    }
} }
"""

def generate_header(header_files, template_data):
    """
    template_data is a dictionary as explained above
    """

    typedefs_list = []
    sizeofs_list = []

    for key, value in template_data.items():
        template_name = key
        for inst_data in value:
            template_arguments, type_name = inst_data
            typedefs_list.append('    typedef %s< %s > %s;' % (template_name, template_arguments, type_name))
            sizeofs_list.append('        sizeof(%s< %s >);' % (template_name, template_arguments))

    headers = '\n'.join(['#include <%s>' % header for header in header_files])
    typedefs = '\n'.join(typedefs_list)
    sizeofs = '\n'.join(sizeofs_list)

    return generated_header_template % (headers, typedefs, sizeofs)


# generate the header file to be passed to gccxml
header_code = generate_header(header_files, template_data)

# write the generated header file to disk
# (needed to get includes in the code generated by py++ right)
try:
    os.mkdir('generated')
except OSError:
    pass
out = file('generated/generated_header.h', 'w')
out.write(header_code)
out.close()

# parse the header file
mb = module_builder_t(files=['generated/generated_header.h'],
                      include_paths=['../../include/'],
                      indexing_suite_version=1
                     )

# by default, do not expose anything
mb.decls().exclude()

# prepare custom lists
decls_to_include = []
decls_to_exclude = []

# include all the requested classes
decls_to_include.extend([mb.class_(cls) for cls in classes])

# include other requested stuff
decls_to_include.extend([mb.decl(decl) for decl in other_stuff])

# apply the requested includes and excludes
for decl in decls_to_include:
   decl.include()
for decl in decls_to_exclude:
    decl.exclude()

# do not expose private and protected members
mb.calldefs(access_type_matcher_t('private')).exclude()
mb.calldefs(access_type_matcher_t('protected')).exclude()

# run the generator
mb.build_code_creator(module_name='faunus')

# write source files to disk
mb.split_module('./generated')
