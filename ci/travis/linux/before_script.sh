# Update translation key files

find src/isimpa -type f -regex '.*/.*\.\(c\|cpp\|h\|hpp\)$' > files.txt
find src/spps -type f -regex '.*/.*\.\(c\|cpp\|h\|hpp\)$' >> files.txt
find src/theorie_classique -type f -regex '.*/.*\.\(c\|cpp\|h\|hpp\)$' >> files.txt
find src/lib_interface -type f -regex '.*/.*\.\(c\|cpp\|h\|hpp\)$' >> files.txt

mkdir -p src/isimpa/lang/
xgettext --keyword=_ --keyword=wxTRANSLATE --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -osrc/isimpa/lang/internat.pot
msginit --no-translator --input src/isimpa/lang/internat.pot -o src/isimpa/lang/internat.pot -l en.UTF-8

find currentRelease/UserScript/job_tool -type f -name "*.py" > files.txt
xgettext --keyword=_  --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -ocurrentRelease/UserScript/job_tool/internat.pot
msginit --no-translator --input currentRelease/UserScript/job_tool/internat.pot -o currentRelease/UserScript/job_tool/internat.pot -l en.UTF-8

find currentRelease/UserScript/moveto_vertex -type f -name "*.py" > files.txt
xgettext --keyword=_  --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -ocurrentRelease/UserScript/moveto_vertex/internat.pot
msginit --no-translator --input currentRelease/UserScript/moveto_vertex/internat.pot -o currentRelease/UserScript/moveto_vertex/internat.pot -l en.UTF-8

find currentRelease/UserScript/preceiv_sourceTracker -type f -name "*.py" > files.txt
xgettext --keyword=_  --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -ocurrentRelease/UserScript/preceiv_sourceTracker/internat.pot
msginit --no-translator --input currentRelease/UserScript/preceiv_sourceTracker/internat.pot -o currentRelease/UserScript/preceiv_sourceTracker/internat.pot -l en.UTF-8

find currentRelease/UserScript/recp_tool -type f -name "*.py" > files.txt
xgettext --keyword=_  --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -ocurrentRelease/UserScript/recp_tool/internat.pot
msginit --no-translator --input currentRelease/UserScript/recp_tool/internat.pot -o currentRelease/UserScript/recp_tool/internat.pot -l en.UTF-8

find currentRelease/UserScript/source_tools -type f -name "*.py" > files.txt
xgettext --keyword=_  --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -ocurrentRelease/UserScript/source_tools/internat.pot
msginit --no-translator --input currentRelease/UserScript/source_tools/internat.pot -o currentRelease/UserScript/source_tools/internat.pot -l en.UTF-8

find currentRelease/UserScript/SppsReportSample -type f -name "*.py" > files.txt
xgettext --keyword=_  --from-code=UTF-8 -s --no-wrap  -no-hash --escape -ffiles.txt -ocurrentRelease/UserScript/SppsReportSample/internat.pot
msginit --no-translator --input currentRelease/UserScript/SppsReportSample/internat.pot -o currentRelease/UserScript/SppsReportSample/internat.pot -l en.UTF-8

# Now replace all ASCII charset by UTF-8 in pot files
find . -type f -name "*.pot" -exec sed -i 's/charset=ASCII/charset=UTF-8/g' {} +

# Push translation keys to transifex

if [ -z "$TRANSIFEXPWD" ]; then
    echo "Not in master branch do not push transifex keys"
else
    # Write transifex config file
    printf "[https://www.transifex.com]\nhostname = https://www.transifex.com\npassword = $TRANSIFEXPWD\ntoken =\nusername = travis_lae\n" > ~/.transifexrc
    # install transifex client
    pip install --user transifex-client
    # push transifex keys
    tx push -s
fi