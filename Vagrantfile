Vagrant.configure(2) do |config|
  config.vm.synced_folder ".", "/mk"
  config.vm.define "jessie64" do |jessie64|
    jessie64.vm.box = "debian/contrib-jessie64"
    jessie64.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    jessie64.vm.provision "shell", inline: <<-SHELL
      cd /mk
      ./build/docker/script/run gcc-jessie depend start_over
      echo "Now run:"
      echo "1. vagrant ssh"
      echo "2. cd /mk"
      echo "3. ./build/vagrant/jessie64"
      echo "4. sudo make install"
    SHELL
  end
end
