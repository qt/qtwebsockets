~/Downloads/cov-analysis-macosx-6.5.1.4/bin/cov-build --dir cov-int /Users/kurtpattyn/Applications/Qt5.1/5.1.0/clang_64/bin/qmake && make
tar czvf QWebSockets.tgz cov-int
curl --form project=QWebSockets --form token=rQC7jP0vAzaKR-lhgLGTdQ --form email=pattyn.kurt@gmail.com --form file=@QWebSockets.tgz --form version=0.1 --form description=Alpha http://scan5.coverity.com/cgi-bin/upload.py
