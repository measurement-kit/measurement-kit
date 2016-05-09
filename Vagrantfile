Vagrant.configure(2) do |config|
  config.vm.synced_folder ".", "/mk"
  config.vm.define "trusty64" do |trusty64|
    trusty64.vm.box = "ubuntu/trusty64"
    trusty64.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    trusty64.vm.provision "shell", inline: <<-SHELL
      sudo apt-get update
      sudo apt-get dist-upgrade -y
      sudo apt-get install -y g++ autoconf automake libtool valgrind git
      sudo apt-get install -y libevent-dev libssl-dev libgeoip-dev
    SHELL
  end
end
