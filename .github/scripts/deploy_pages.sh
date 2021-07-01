#!/bin/bash

set -e -x

tar -xf html.tar.gz
rm -f html.tar.gz
git checkout gh-pages
find * -maxdepth 0 -not -path "html*" -exec rm -rf {} \;
cp -r html/* .
rm -rf html

touch .nojekyll # https://github.blog/2009-12-29-bypassing-jekyll-on-github-pages/

git config --local user.email "Valentina.Kats@intel.com"
git config --local user.name "Valentina Kats"
git remote set-url origin https://x-access-token:${GITHUB_TOKEN}@github.com/${GITHUB_REPOSITORY}
git add .
git commit --amend -am "Publishing on pages for ${GITHUB_SHA:1:8}" -s
git push -f
