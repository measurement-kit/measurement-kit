Vagrant.configure(2) do |config|
  config.vm.synced_folder ".", "/mk"
  config.vm.define "yakkety" do |yakkety|
    yakkety.vm.box = "ubuntu/yakkety64"
    yakkety.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
  end
end
