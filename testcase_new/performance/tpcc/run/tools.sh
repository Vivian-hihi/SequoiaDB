#!/bin/bash


function getHosts()
{
   cd data
   hosts=()
   for dirent in $(dir)
   do
      if [ -d $dirent ];then
         hosts=(${hosts[*]} $dirent)
      fi
   done
   cd .. 
   echo ${hosts[*]}
}

function getHardWareItemByName()
{
   if [ $# -eq 2 ];then
      hardwareinfo=data/$1/hardware.txt
   else
      hardwareinfo=data/hardware.txt
   fi
   
   if [ ! -f "${hardwareinfo}" ];then
      echo "${hardwareinfo} not exist"
      return
   fi 
   itemName=$2

   cat ${hardwareinfo}|awk -F '=' '/'"${itemName}"'/{print $2}'
   
}

function getSoftWareItemByName()
{

   if [ $# -eq 2 ];then
      softwareinfo=data/$1/software.txt
   else
      softwareinfo=data/software.txt
   fi
   
   if [ ! -f "${softwareinfo}" ];then
      echo "${softwareinfo} not exist"
      return
   fi

   itemName=$2
   cat ${softwareinfo} | awk -F '=' '/'"${itemName}"'/{print $2}'
}

function getCpuModeName()
{
   getHardWareItemByName $1 "model name"
}

function getPhysicalCpuNum()
{
   getHardWareItemByName $1 "Physical number"
}

function getCoreNumPerCpu()
{
   getHardWareItemByName $1 "Core\(s\) per socket"
}

function getThreadNumPerCore()
{
   getHardWareItemByName $1 "Thread\(s\) per core"
}

function getTotalMemoryNum()
{
   getHardWareItemByName $1 "MemTotal"
}

function getNicInfo()
{
   getHardWareItemByName $1 "NIC"
}

function getDiskCap()
{
   getHardWareItemByName $1 "Disk capacity"
}

function getOsRelease()
{
   getSoftWareItemByName $1 "OS Release"
}

function getKernelVersion()
{
   getSoftWareItemByName $1 "kernel"
}

function getSequoiaDBVersion()
{
   getSoftWareItemByName $1 "SequoiaDB version"
}

function getPostgreSQLVersion()
{
   getSoftWareItemByName $1 "PostgreSQL"
}

