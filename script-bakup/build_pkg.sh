SEP_VERSION=$1
TOOLS="BoMC DSA UXLite CBB Toolkit"

OS_DISTRO_x32="WIN SLES11 SLES10 SLES9 RHEL6 RHEL5 RHEL4 RHEL3"
OS_DISTRO_x64="WIN64 SLES11_64 SLES10_64 SLES9_64 RHEL6_64 RHEL5_64 RHEL4_64 RHEL3_64 MCP"
README_SEP="sep_readme.txt"
HISTORY="xml_tml/sep_history.chg"

mkdir output
mkdir tmp

#clean US directory
mkdir ../US
rm -rf ../US/*


#sj: Merge CBB candidate for WIN64 and candidate for WIN into ONE file.
#rhb: removed block for defect 498625
#

for os in $OS_DISTRO_x32
do
	rm -rf tmp/*
	for tool in $TOOLS
	do
		if [ -d $tool/$os ]; then
			cp -vf $tool/$os/*.zip tmp
		fi
	done
	
	if [ -d docs ]; then
         cp -rvf docs/* tmp/
	fi

	cd tmp
	for zipfile in `ls *.zip`
	do
		unzip -o $zipfile
		rm -rf $zipfile
	done
	cd -

	FDR_OS=$(echo $os | tr '[A-Z]' '[a-z]')
	readme=xml_tml/$README_SEP
	if [ $FDR_OS == "win" ]; then
		FDR_OS=windows
	fi
  base=ibm_utl_tsep_${SEP_VERSION}_${FDR_OS}_i386
	cp -vf $readme output/$base.txt
  cp -vf $HISTORY output/$base.chg
	pkgname=$base.zip

	cd tmp
	zip -q -r $pkgname .
	mv $pkgname ../output	
	cd -
done

for os in $OS_DISTRO_x64
do
	rm -rf tmp/*
	for tool in $TOOLS
	do
		if [ -d $tool/$os ]; then
			cp -vf $tool/${os}/*.zip tmp
		fi
	done

	#  special  temporary for win64
	#rhb: removed block for defect 498625
	#

	if [ -d docs ]; then
         cp -rvf docs/* tmp/
	fi

	cd tmp
	for zipfile in `ls *.zip`
	do
		unzip -o $zipfile
		rm -rf $zipfile
	done
	cd -

	
	FDR_OS=$(echo $os | tr '[A-Z]' '[a-z]')
  readme=xml_tml/$README_SEP
	if [ $FDR_OS == "win64" ]; then
		FDR_OS=windows
	fi
	if [ $FDR_OS == "mcp" ]; then
		FDR_OS=anyos
	fi
	FDR_OS=`echo $FDR_OS | sed -e "s%_64%%"`
        base=ibm_utl_tsep_${SEP_VERSION}_${FDR_OS}_x86-64
	pkgname=$base.zip
	cp -vf $readme output/$base.txt
        cp -vf $HISTORY output/$base.chg

	cd tmp
	zip -q -r $pkgname .
	mv $pkgname ../output
	cd -
done



# seperate process for winpe
#winpe i386
WINPE_FILE="Toolkit/winpe/*.zip"
rm -rf tmp/*
cp -vf $WINPE_FILE tmp



if [ $? == 0 ]; then
	cd tmp
	zipfile=`ls`
	unzip -o $zipfile
	rm -rf $zipfile
	cd -
fi

if [ -d docs ]; then
 cp -rvf docs/* tmp/
fi

cp -vf xml_tml/$README_SEP output/ibm_utl_tsep_${SEP_VERSION}_winpe_i386.txt
cp -vf $HISTORY output/ibm_utl_tsep_${SEP_VERSION}_winpe_i386.chg
pkgname=ibm_utl_tsep_${SEP_VERSION}_winpe_i386.zip
cd tmp
zip -q -r $pkgname .
mv $pkgname ../output
cd -

#winpe x64
WINPE_FILE="Toolkit/winpe_64/*.zip"
rm -rf tmp/*
cp -vf $WINPE_FILE tmp



if [ $? == 0 ]; then
	cd tmp
	zipfile=`ls`
	unzip -o $zipfile
	rm -rf $zipfile
	cd -
fi

if [ -d docs ]; then
 cp -rvf docs/* tmp/
fi

cp -vf xml_tml/$README_SEP output/ibm_utl_tsep_${SEP_VERSION}_winpe_x86-64.txt
cp -vf $HISTORY output/ibm_utl_tsep_${SEP_VERSION}_winpe_x86-64.chg
pkgname=ibm_utl_tsep_${SEP_VERSION}_winpe_x86-64.zip
cd tmp
zip -q -r $pkgname .
mv $pkgname ../output
cd -

# copy to US folder
mv -f output/* ../US

