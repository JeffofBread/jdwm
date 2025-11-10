project('jdwm', 'c', version : '0.2.00', meson_version : '>=0.63.0', default_options : ['c_std=c99', 'warning_level=1'])

############################################################################
# Git project version
############################################################################

version_string = meson.project_version()

# Attempt to automatically get git branch/commit hash
git = find_program('git', required : false)
if git.found()
        git_branch_command = run_command('git', 'rev-parse', '--abbrev-ref', 'HEAD', check : false)
        if git_branch_command.returncode() == 0
                git_branch = git_branch_command.stdout().strip()
        endif

        git_hash_command = run_command('git', 'rev-parse', '--short=7', 'HEAD', check : false, )
        if git_hash_command.returncode() == 0
                git_hash = git_hash_command.stdout().strip()
        endif
endif

# Override git branch from CLI
if get_option('git_branch') != ''
        git_branch = get_option('git_branch')
endif

# Override git hash from CLI
if get_option('git_hash') != ''
        git_hash = get_option('git_hash')
endif

# If a branch was defined, add to version_string
if is_variable('git_branch')
        version_string = version_string + '-' + git_branch
endif

# If a hash was defined, add to version_string
if is_variable('git_hash')
        version_string = version_string + '-' + git_hash
endif

message('Version string set to "' + version_string + '"')

############################################################################
# Compile arguments
############################################################################

cflags = [
        '-D_XOPEN_SOURCE=700L',
        '-DVERSION="@0@"'.format(version_string),
]

if get_option('buildtype') in ['debug', 'debugoptimized']
        cflags += ['-D_DEBUG']
endif

if host_machine.system() == 'linux'
        cflags += ['-D_DEFAULT_SOURCE']
endif

############################################################################
# Files
############################################################################

source_files = [
        files(
                'source/atoms.c',
                'source/bar.c',
                'source/cli.c',
                'source/client.c',
                'source/core.c',
                'source/draw.c',
                'source/gaps.c',
                'source/layout.c',
                'source/logger.c',
                'source/main.c',
                'source/monitor.c',
                'source/parser.c',
                'source/systray.c',
                'source/tags.c',
                'source/util.c',
                'source/xevent.c',
                'source/xproperty.c',
                'source/xutil.c',
        )
]

include_directory = include_directories('include')

############################################################################
# Dependencies
############################################################################

# TODO: Make imlib2 icon rendering in the bar optional

dependencies = []
required_dependencies = [
        'x11',
        'xft',
        'fontconfig',
        'xrender',
        'imlib2',
        'libconfig',
]

foreach i : required_dependencies
        dependencies += [dependency(i, required : not get_option('disable_deps'))]
endforeach

xinerama = dependency('xinerama', required : get_option('enable_xinerama'))
if xinerama.found()
        add_project_arguments('-DXINERAMA', language : 'c')
        dependencies += [xinerama]
else
        warning('xinerama not found.')
endif

# Use local gnu getopt subproject
gnu_getopt = subproject('gnu_getopt')
dependencies += gnu_getopt.get_variable('gnu_getopt_dependency')

############################################################################
# Install Manuals
############################################################################

install_man(
        files(
                'resources/jdwm.1',
        ),
)

############################################################################
# Install Default Configuration
############################################################################

install_data(
        files(
                'resources/jdwm.conf',
        ),
        install_dir : '/etc/jdwm/', # TODO: Look into a possibly better way to define these paths
)

############################################################################
# Install Desktop File
############################################################################

install_data(
        files(
                'resources/jdwm.desktop',
        ),
        install_dir : '/usr/share/xsessions/', # TODO: Look into a possibly better way to define these paths
)

############################################################################
# Executable Creation
############################################################################

jdwm = executable(
        meson.project_name(),
        source_files,
        c_args : cflags + ['-DPROGRAM_NAME="@0@"'.format(meson.project_name())],
        dependencies : dependencies,
        include_directories : include_directory,
        gnu_symbol_visibility : 'hidden',
        install : true,
)
