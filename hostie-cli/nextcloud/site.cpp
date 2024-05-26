#include "../environment.hpp"
#include "../httpd.hpp"

using namespace std;
using namespace HttpServer;

namespace NextCloud
{
  void site_initializer(const InstanceEnvironment& environment, Site& site)
  {
    //site.protocol = "http2";
    site.custom_settings = {
      "index index.php index.html /index.php$request_uri;",
      "server_tokens off;",
      "client_max_body_size 512M;",
      "client_body_timeout 300s;",
      "client_body_buffer_size 512k;"
      "fastcgi_buffers 64 4K;",
      "",
      "add_header Referrer-Policy                   \"no-referrer\"       always;",
      "add_header Strict-Transport-Security         \"max-age=31536000\"  always;",
      "add_header X-Content-Type-Options            \"nosniff\"           always;",
      "add_header X-Frame-Options                   \"SAMEORIGIN\"        always;",
      "add_header X-Permitted-Cross-Domain-Policies \"none\"              always;",
      "add_header X-Robots-Tag                      \"noindex, nofollow\" always;",
      "add_header X-XSS-Protection                  \"1; mode=block\"     always;",
      "fastcgi_hide_header X-Powered-By;",
      "",
      "gzip on;",
      "gzip_vary on;",
      "gzip_comp_level 4;",
      "gzip_min_length 256;",
      "gzip_proxied expired no-cache no-store private no_last_modified no_etag auth;",
      "gzip_types application/atom+xml text/javascript application/javascript application/json application/ld+json application/manifest+json application/rss+xml application/vnd.geo+json application/vnd.ms-fontobject application/wasm application/x-font-ttf application/x-web-app-manifest+json application/xhtml+xml application/xml font/opentype image/bmp image/svg+xml image/x-icon text/cache-manifest text/css text/plain text/vcard text/vnd.rim.location.xloc text/vtt text/x-component text/x-cross-domain-policy;",
      "",
      "include mime.types;",
      "types {",
      "  text/javascript js mjs;",
	    "  application/wasm wasm;",
      "}"
    };

    site.locations.push_back(Location{
      "= /robots.txt", {}, CustomLocation, NoSslState,
      {"allow all;", "log_not_found off;", "access_log off;"}
    });

    site.locations.push_back(Location{
      "^~ /.well-known", {}, CustomLocation, SslOnly,
      {
        "location = /.well-known/carddav { return 301 /remote.php/dav; }",
        "location = /.well-known/caldav  { return 301 /remote.php/dav; }",
        "location /.well-known/acme-challenge    { try_files $uri $uri/ =404; }",
        "location /.well-known/pki-validation    { try_files $uri $uri/ =404; }",
        "return 301 /index.php$request_uri;"
      }
    });

    site.locations.push_back(Location{
      "~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/)", {}, CustomLocation, SslOnly,
      {"return 404;"}
    });

    site.locations.push_back(Location{
      "~ ^/(?:\\.|autotest|occ|issue|indie|db_|console)", {}, CustomLocation, SslOnly,
      {"return 404;"}
    });

    site.locations.push_back(Location{
      "~ \\.php(?:$|/)", {}, CustomLocation, SslOnly,
      {
        "rewrite ^/(?!index|remote|public|cron|core\\/ajax\\/update|status|ocs\\/v[12]|updater\\/.+|ocs-provider\\/.+|.+\\/richdocumentscode(_arm64)?\\/proxy) /index.php$request_uri;"
        "",
        "fastcgi_split_path_info ^(.+?\\.php)(/.*)$;",
        "set $path_info $fastcgi_path_info;",
        "",
        "try_files $fastcgi_script_name =404;",
        "",
        "include fastcgi_params;",
        "fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;",
        "fastcgi_param PATH_INFO $path_info;",
        "fastcgi_param HTTPS on;",
        "fastcgi_param modHeadersAvailable true;",
        "fastcgi_param front_controller_active true;",
        "fastcgi_pass unix:" + environment.get_variable("PHP_FPM_SOCKET") + ';',
        "fastcgi_intercept_errors on;",
        "fastcgi_request_buffering off;",
        "fastcgi_max_temp_file_size 0;"
      }
    });

    site.locations.push_back(Location{
      "~ \\.(?:css|js|mjs|svg|gif|png|jpg|ico|wasm|tflite|map|ogg|flac)$", {}, CustomLocation, SslOnly,
      {
        "try_files $uri /index.php$request_uri;",
        "add_header Cache-Control                     \"public, max-age=15778463\";",
        "add_header Referrer-Policy                   \"no-referrer\"       always;",
        "add_header X-Content-Type-Options            \"nosniff\"           always;",
        "add_header X-Frame-Options                   \"SAMEORIGIN\"        always;",
        "add_header X-Permitted-Cross-Domain-Policies \"none\"              always;",
        "add_header X-Robots-Tag                      \"noindex, nofollow\" always;",
        "add_header X-XSS-Protection                  \"1; mode=block\"     always;",
        "access_log off;"
      }
    });

    site.locations.push_back(Location{
      "~ \\.woff2?$", {}, CustomLocation, SslOnly,
      {
        "try_files $uri /index.php$request_uri;",
        "expires 7d;",
        "access_log off;"
      }
    });

    site.locations.push_back(Location{
      "/remote", "/remote.php$request_uri", RedirectLocation, SslOnly
    });

    site.locations.push_back(Location{
      "/", {}, CustomLocation, SslRequired, {"try_files $uri $uri/ /index.php$request_uri;"}
    });
  }
}

