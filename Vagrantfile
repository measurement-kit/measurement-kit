Vagrant.configure(2) do |config|
  config.vm.synced_folder ".", "/mk"
  config.vm.define "zesty" do |zesty|
    zesty.vm.box = "ubuntu/zesty64"
    zesty.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
  end
end
