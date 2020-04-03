# FIPS status check module for NGINX

## Introduction

This module applies to NGINX builds that use OpenSSL for SSL/TLS crypto.  It runs after 
NGINX startup and queries the OpenSSL library, reporting if the library is in FIPS mode or not.

```sh
sudo tail /var/log/nginx/error.log
2020/04/03 07:45:54 [notice] 11250#11250: using the "epoll" event method
2020/04/03 07:45:54 [notice] 11250#11250: OpenSSL FIPS Mode is enabled
2020/04/03 07:45:54 [notice] 11250#11250: nginx/1.17.6 (nginx-plus-r20)
2020/04/03 07:45:54 [notice] 11250#11250: built by gcc 4.8.5 20150623 (Red Hat 4.8.5-36) (GCC)
2020/04/03 07:45:54 [notice] 11250#11250: OS: Linux 3.10.0-1062.el7.x86_64
```

For more information on using NGINX in FIPS mode, see the [NGINX Plus FIPS documentation], which applies to both NGINX open source builds and NGINX Plus. To determine which TLS ciphers NGINX offers, the [nmap ssl-enum-ciphers] script is useful.

  [NGINX Plus FIPS documentation]:https://docs.nginx.com/nginx/fips-compliance-nginx-plus/
  [nmap ssl-enum-ciphers]:https://nmap.org/nsedoc/scripts/ssl-enum-ciphers.html

## Installation

Build the module as an [NGINX Dynamic Module], against the corresponding NGINX base version, then load it in your NGINX configuration.

  [NGINX Dynamic Module]:https://www.nginx.com/blog/compiling-dynamic-modules-nginx-plus/
  
 **Obtain the base NGINX source**

Determine the base version for your NGINX build:

```sh
$ nginx -v
nginx version: nginx/1.17.6 (nginx-plus-r20)
```

Obtain the corresponding NGINX source, and untar it into the same location as this repository:

```sh
$ wget https://nginx.org/download/nginx-1.17.6.tar.gz
$ tar -xzvf nginx-1.17.6.tar.gz
$ ls -1F
nginx-1.17.6/
nginx-1.17.6.tar.gz
nginx-fips-check-module/
```

**Build the module**

Note: ensure you have the necessary build tools, and the openssl-dev (or equivalent) packages installed.

The following `configure` command should build modules correctly for NGINX Plus and the standard NGINX open source binary releases.  If the resulting module does not load into your NGINX binary, check the `configure` options and the output of `nginx -V`.

```sh
$ cd nginx-1.17.6
$ ./configure --with-compat --with-http_ssl_module --add-dynamic-module=../nginx-fips-check-module
$ make modules
```

Copy the module to the standard NGINX module location:

```sh
$ sudo cp objs/nginx_fips_enabled_module.so /etc/nginx/modules/
```

**Configure the module**

Add the following to the beginning of `/etc/nginx/nginx.conf`:

```
load_module modules/ngx_fips_check_module.so;
```

**Check FIPS status**

Restart nginx and check the error log `/var/log/nginx/error.log`:

```sh
$ sudo nginx -s stop ; sudo nginx ; sudo tail /var/log/nginx/error.log
2020/04/03 08:21:55 [notice] 11251#11251: worker process 11253 exited with code 0
2020/04/03 08:21:55 [notice] 11251#11251: exit
2020/04/03 08:21:55 [notice] 13723#13723: using the "epoll" event method
2020/04/03 08:21:55 [notice] 13723#13723: OpenSSL FIPS Mode is enabled
2020/04/03 08:21:55 [notice] 13723#13723: nginx/1.17.6 (nginx-plus-r20)
2020/04/03 08:21:55 [notice] 13723#13723: built by gcc 4.8.5 20150623 (Red Hat 4.8.5-36) (GCC)
2020/04/03 08:21:55 [notice] 13723#13723: OS: Linux 3.10.0-1062.el7.x86_64
2020/04/03 08:21:55 [notice] 13723#13723: getrlimit(RLIMIT_NOFILE): 1024:4096
2020/04/03 08:21:55 [notice] 13724#13724: start worker processes
2020/04/03 08:21:55 [notice] 13724#13724: start worker process 13726
```

