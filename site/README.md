# IntelBurnTest website

This is the static website for IntelBurnTest ASM. The page uses plain HTML and
CSS; no build step or JavaScript is needed. Its local entry point is
[`index.html`](index.html).

GitHub Pages publishes the root of the `gh-pages` branch. The branch is a
subtree of this directory, so the website can be maintained with the rest of
the application without occupying `/docs` on `main`.

After editing the site on `main`, publish it with:

```console
git subtree push --prefix site origin gh-pages
```

The project URL is <https://thejuampi.github.io/ibt-asm/>.

When publishing a new release, update the version, binary sizes, and direct
download links in `index.html` against the actual GitHub Release. Also verify
that the screenshots and their captions still describe the current UI.
