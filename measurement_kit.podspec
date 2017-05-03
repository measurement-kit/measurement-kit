Pod::Spec.new do |s|
  s.name = "measurement_kit"
  s.version = "0.6.0-beta"
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
  s.default_subspec = 'Binary'
  s.subspec 'Source' do |x|
    x.source = {
      :git => "https://github.com/measurement-kit/measurement-kit.git",
      :tag => "v#{s.version}"
    }
    x.prepare_command = <<-CMD
      ./build/ios/library
    CMD
    x.vendored_framework = "build/ios/Frameworks/*.framework"
  done
  s.subspec 'Binary' do |x|
    x.source = {
      :http => https://github.com/measurement-kit/measurement-kit/releases/download/v0.6.0-beta/ios_frameworks-0.6.0-beta.zip
    }
    x.vendored_framework = "build/ios/Frameworks/*.framework"
  done
  s.platform = :ios, "7.1"
end
