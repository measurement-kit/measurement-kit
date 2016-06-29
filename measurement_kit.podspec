Pod::Spec.new do |s|
  s.name = "measurement_kit"
  s.version = "0.2.0"
  s.summary = "Portable network measurement library"
  s.author = "Davide Allavena",
             "Simone Basso",
             "Arturo Filasto'",
             "Antonio Langiu",
             "Lorenzo Primiterra",
             "Alessandro Quaranta"
  s.homepage = "https://github.com/measurement-kit"
  s.license = { :type => "BSD" }
  s.source = {
    :git => "https://github.com/measurement-kit/measurement-kit.git",
    :tag => "v#{s.version}"
  }
  s.prepare_command = <<-CMD
    ./build/ios/scripts/build.sh
  CMD
  s.platform = :ios, "7.1"
  s.vendored_framework = "build/ios/Frameworks/*.framework"
end
