# fetchpkg
This is a simple [cURL][curl]-based tool for downloading official Sony
Playstation 4 & 5 game updates. It is available for GNU/Linux, Windows (64bit),
and jailbroken PS5s. Game updates that are plit across multiple files are merged
automatically during download. If you are behind a firewall, you can download
updates via a proxy server, which is convenient when running on a jailbroken
PS5 that are shielded of from the internet.


## Usage
```console
john@localhost:ps5-payload-dev/fetchpkg$ ./fetchpkg [-p PROXY] [-o PATH] URL 
```
where URL is the address to a JSON-formatted manifest, e.g., the PS4 title
[Uncharted: The Lost Legacy (v1.09)][CUSA09564].

## PS5 usage example
To run fetchpkg on the PS5, you need to load it with an ELF loader that provides
a command line interface, e.g., [shsrv][shsrv], and the means to transfer the ELF
to the ps5, e.g., [ftpsrv][ftpsrv]. The following commands will upload the ELF file
to /data, connect to shsrv, and download a pkg file to /mnt/usb0 via a proxy server:
```console
john@localhost:ps5-payload-dev/fetchpkg$ curl -T fetchpkg.elf ftp://ps5:2121/data/fetchpkg.elf
john@localhost:ps5-payload-dev/fetchpkg$ telnet ps5 2323
Welcome to shsrv.elf running on...
...
/$ /data/fetchpkg.elf -p http://proxy:8080 -o /mnt/usb0/temp.pkg URL
```

## Limitations
Most PS4 updates seems to be hosted via HTTP, while PS5 updates are typically
HTTPS. Unfortunately, there is currently a bug when combining the latest versions 
of cURL with the cryptographic library mbedtls used by the PS5 port of fetchpkg, 
see https://github.com/curl/curl/issues/13653 
and https://github.com/Mbed-TLS/mbedtls/issues/9210
for more details. Consequently, the current PS5 port cannot download game updates 
that are hosted on HTTPS servers.

## Building
See the [gihub CI action workflow][workflow].

## Reporting Bugs
If you encounter problems with fetchpkg, please [file a github issue][issues].
If you plan on sending pull requests which affect more than a few lines of code,
please file an issue before you start to work on you changes. This will allow us
to discuss the solution properly before you commit time and effort.

## License
fetchpkg is licensed under the GPLv3+, and uses [cURL][curl] togeather with
[parson][parsonurl], both published under MIT-like licences. See thier
websites for further details.

[shsrv]: https://github.com/ps5-payload-dev/shsrv
[ftpsrv]: https://github.com/ps5-payload-dev/ftpsrv
[curl]: https://curl.se
[parsonurl]: http://kgabis.github.io/parson
[issues]: https://github.com/ps5-payload-dev/fetchpkg/issues/new
[workflow]: https://github.com/ps5-payload-dev/fetchpkg/blob/master/.github/workflows/ci.yml
[CUSA09564]: http://gs2.ww.prod.dl.playstation.net/gs2/ppkgo/prod/CUSA09564_00/9/f_f981fc66c3e96296bbb5aab1a93ef0615f7e91a15ef5b824689715494c33c0fb/f/EP9000-CUSA09564_00-UNCHD4LOSTLEGACY-A0109-V0100.json
