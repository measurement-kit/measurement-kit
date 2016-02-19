Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.synced_folder ".", "/vagrant", disabled: true
  config.vm.provider "virtualbox" do |v|
    v.memory = 2048
  end
  config.vm.provision "shell", inline: <<-SHELL
    APT_PACKAGES="autoconf g++ git libtool libevent-dev libssl-dev valgrind"
    set -e
    sudo apt-get install -y $APT_PACKAGES                                   && \
      git clone https://github.com/measurement-kit/measurement-kit          && \
      cd measurement-kit                                                    && \
      ./autogen.sh                                                          && \
      ./configure                                                           && \
      make V=0                                                              && \
      make check V=0
  SHELL
end
