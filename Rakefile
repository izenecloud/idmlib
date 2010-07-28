# Wrapper of CMAKE
[
 File.join(File.dirname(__FILE__), "../cmake/lib"), # in same top directory
 File.join(File.dirname(__FILE__), "../../cmake--master/workspace/lib") # for hudson
].each do |dir|
  if File.directory? dir
    $: << dir
    ENV["EXTRA_CMAKE_MODULES_DIRS"] = File.dirname(File.expand_path(dir))
  end
end

require "izenesoft/project-finder"
# Must find default dependent projects before require izenesoft/tasks,
# which will load env.yml or cmake.yml and override the values found here.
finder = IZENESOFT::ProjectFinder.new(File.dirname(__FILE__))
finder.find_izenelib
finder.find_ilplib
finder.find_imllib
finder.find_libxml2
finder.find_kma
finder.find_icma

require "izenesoft/tasks"

task :default => :cmake

IZENESOFT::CMake.new do |t|
  t.source_dir = "."
end

IZENESOFT::GITClean.new

task :env do
  sh "/usr/bin/env"
end
