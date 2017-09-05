Pod::Spec.new do |s|
  s.name = "measurement_kit"
  s.version = "0.7.1"
  s.summary = "Portable network measurement library"
  s.author = "Simone Basso",
             "Arturo Filastò",
             "Davide Allavena",
             "Carmine D'Amico",
             "Leonid Evdokimov",
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
    ./build/ios/library
  CMD
  s.platform = :ios, "9.0"
  s.vendored_framework = "build/ios/Frameworks/*.framework"
end
