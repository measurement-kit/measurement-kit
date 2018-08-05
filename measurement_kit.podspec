Pod::Spec.new do |s|
  s.name = "measurement_kit"
  s.version = "0.9.0-alpha.6"
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
    echo ""
    echo "*** BUILD ERROR ***"
    echo ""
    echo "Please, from now one use https://github.com/measurement-kit/ios-libs"
    echo "It should suffice to update the repository name in your Podfile"
    echo ""
    echo ""
    exit 1
  CMD
  s.platform = :ios, "9.0"
  s.vendored_framework = "build/ios/Frameworks/*.framework"
end
