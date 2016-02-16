# this tool should be copied outside trunk directory
# usage: ./upload.sh <svn version>
#!/bin/sh
#BASERELEASE="trunk"
BASERELEASE="engine_1.12"
if [[ $# == 1 ]];then
   if [[ $1 == "init" ]];then
      #initialize local config
      git config--globaluser.name "Tao Wang"
      git config--globaluser.email "taoewang@sequoiadb.com"

      #initialize local repository
      rm -rf ~/github
      mkdir ~/github
      cd ~/github
      git clone git@github.com:SequoiaDB/SequoiaDB.git
      rm -rf ~/csdn
      mkdir ~/csdn
      cd ~/csdn
      git clone git@code.csdn.net:SequoiaDB/sequoiadb.git
      rm -rf ~/osc
      mkdir ~/osc
      cd ~/osc
      git clone git@git.oschina.net:wangzhonnew/SequoiaDB.git
      rm -rf ~/gitcafe
      mkdir ~/gitcafe
      cd ~/gitcafe
      git clone git@gitcafe.com:SequoiaDB/SequoiaDB.git
      exit
   fi
fi
# svn code checkout
cd ~/$BASERELEASE
#if [[ $# == 1 ]];then
 #  svn update -r $1
#else
 #  svn update
#fi
gittools/clone.sh /home/taoewang/github/SequoiaDB
gittools/clone.sh /home/taoewang/csdn/sequoiadb
gittools/clone.sh /home/taoewang/osc/SequoiaDB
gittools/clone.sh /home/taoewang/gitcafe/SequoiaDB

# git commit
cd ~/$BASERELEASE
svn_version=`svn info | grep Revision`
cd ~/github/SequoiaDB
git pull
git add .
git commit -m "$svn_version"
git push
cd ~/csdn/sequoiadb
git pull
git add .
git commit -m "$svn_version"
git push
cd ~/osc/SequoiaDB
git pull
git add .
git commit -m "$svn_version"
git push
cd ~/gitcafe/SequoiaDB
git pull
git add .
git commit -m "$svn_version"
git push
