Pod::Spec.new do |s|
  s.name = "measurement-kit"
  s.version = "0.1.0"
  s.summary = "Portable network measurement library"
  s.author = "Simone Basso <bassosimone@gmail.com>"
  s.homepage = "https://github.com/measurement-kit"
  s.license = { :type => "BSD", :file => "LICENSE" }
  s.source = {
    :git => "https://github.com/measurement-kit/measurement-kit.git",
    :branch => "master"
  }
  s.prepare_command = <<-CMD
    cd mobile/ios/ && ./scripts/build.sh
  CMD
  s.platform = :ios, "9.0"
  s.vendored_framework = "mobile/ios/Frameworks/*.framework"
end
