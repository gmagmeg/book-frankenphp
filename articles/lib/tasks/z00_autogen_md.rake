# frozen_string_literal: true

namespace :autogen do
  desc 'Generate .re file that includes all markdown files in articles/md'
  task :md_chapters do
    md_files = Dir.glob('md/*.md').sort
    generated_re_file = '_autogen_md_chapters.re'

    content = md_files.map do |md_file|
      "//mdinclude[#{md_file}]"
    end.join("\n")

    # The content should be written only when it is changed.
    # Because the file modification time is used as a trigger for the build process.
    if !File.exist?(generated_re_file) || File.read(generated_re_file) != content
      File.write(generated_re_file, content + "\n")
    end
  end
end

# Add dependency to main build tasks
%i[pdf epub web text plaintext idgxml vivliostyle].each do |task_name|
  if Rake::Task.task_defined?(task_name)
  Rake::Task[task_name].prerequisites.unshift('autogen:md_chapters')
end
end

# Add the generated file to the clean task
CLEAN.include('_autogen_md_chapters.re')
