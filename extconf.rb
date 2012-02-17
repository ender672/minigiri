ENV['RC_ARCHS'] = '' if RUBY_PLATFORM =~ /darwin/

# :stopdoc:

require 'mkmf'

RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

ROOT = File.expand_path(File.join(File.dirname(__FILE__), '..', '..'))
LIBDIR = RbConfig::CONFIG['libdir']
INCLUDEDIR = RbConfig::CONFIG['includedir']

if defined?(RUBY_ENGINE) && RUBY_ENGINE == 'macruby'
  $LIBRUBYARG_STATIC.gsub!(/-static/, '')
end

$CFLAGS << " #{ENV["CFLAGS"]}"
$LIBS << " #{ENV["LIBS"]}"

if RbConfig::CONFIG['target_os'] == 'mingw32' || RbConfig::CONFIG['target_os'] =~ /mswin32/
  $CFLAGS << " -DXP_WIN -DXP_WIN32 -DUSE_INCLUDED_VASPRINTF"
end

if RbConfig::MAKEFILE_CONFIG['CC'] =~ /mingw/
  $CFLAGS << " -DIN_LIBXML"
end

if RbConfig::MAKEFILE_CONFIG['CC'] =~ /gcc/
  $CFLAGS << " -Wall -Wcast-qual -Wwrite-strings -Wconversion -Wmissing-noreturn -Winline"
end

if RbConfig::CONFIG['target_os'] =~ /mswin32/
  lib_prefix = 'lib'

  # There's no default include/lib dir on Windows. Let's just add the Ruby ones
  # and resort on the search path specified by INCLUDE and LIB environment
  # variables
  HEADER_DIRS = [INCLUDEDIR]
  LIB_DIRS = [LIBDIR]
  XML2_HEADER_DIRS = [File.join(INCLUDEDIR, "libxml2"), INCLUDEDIR]

else
  lib_prefix = ''

  HEADER_DIRS = [
    # First search /opt/local for macports
    '/opt/local/include',

    # Then search /usr/local for people that installed from source
    '/usr/local/include',

    # Check the ruby install locations
    INCLUDEDIR,

    # Finally fall back to /usr
    '/usr/include',
    '/usr/include/libxml2',
  ]

  LIB_DIRS = [
    # First search /opt/local for macports
    '/opt/local/lib',

    # Then search /usr/local for people that installed from source
    '/usr/local/lib',

    # Check the ruby install locations
    LIBDIR,

    # Finally fall back to /usr
    '/usr/lib',
  ]

  XML2_HEADER_DIRS = [
    '/opt/local/include/libxml2',
    '/usr/local/include/libxml2',
    File.join(INCLUDEDIR, "libxml2")
  ] + HEADER_DIRS

  # If the user has homebrew installed, use the libxml2 inside homebrew
  brew_prefix = `brew --prefix libxml2 2> /dev/null`.chomp
  unless brew_prefix.empty?
    LIB_DIRS.unshift File.join(brew_prefix, 'lib')
    XML2_HEADER_DIRS.unshift File.join(brew_prefix, 'include/libxml2')
  end
end

dir_config('xml2', XML2_HEADER_DIRS, LIB_DIRS)

def asplode(lib)
  abort "-----\n#{lib} is missing.  please visit http://nokogiri.org/tutorials/installing_nokogiri.html for help with installing dependencies.\n-----"
end

asplode "libxml2"  unless find_header('libxml/parser.h')
asplode "libxml2"  unless find_library("#{lib_prefix}xml2", 'xmlParseDoc')

create_makefile('minigiri')
