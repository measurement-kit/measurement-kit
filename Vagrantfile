Vagrant.configure(2) do |config|
  config.vm.synced_folder ".", "/mk"
  config.vm.define "trusty64" do |trusty64|
    trusty64.vm.box = "ubuntu/trusty64"
    trusty64.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    trusty64.vm.provision "shell", inline: <<-SHELL
      cd /mk
      ./build/docker/script/run gcc-trusty depend start_over
      echo "Now run:"
      echo "1. vagrant ssh"
      echo "2. cd /mk"
      echo "3. ./build/vagrant/trusty64"
    SHELL
  end
end
