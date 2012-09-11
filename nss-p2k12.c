/* compile: gcc nss-p2k12.c -shared -lcurl -o /lib/libnss_p2k12.so.2 */
/* install: add "p2k12" to "passwd" and "group" lines in /etc/nsswitch.conf */

#include <errno.h>
#include <nss.h>
#include <pthread.h>
#include <pwd.h>
#include <grp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <curl/easy.h>

#define COPY_IF_ROOM(s) \
    ({ size_t len_ = strlen (s) + 1;                 \
          char *start_ = cp;                         \
          buflen - (cp - buffer) < len_              \
          ? NULL                                     \
          : (cp = mempcpy (cp, s, len_), start_); })

static FILE *
uri_fetch (const char *uri);

/************************************************************************/

static pthread_mutex_t pwd_lock = PTHREAD_MUTEX_INITIALIZER;

static FILE *pwd_file;

enum nss_status
_nss_p2k12_setpwent (int stayopen)
{
  if (pwd_file)
    {
      pthread_mutex_lock (&pwd_lock);
      rewind (pwd_file);
      pthread_mutex_unlock (&pwd_lock);
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_p2k12_endpwent (void)
{
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_p2k12_getpwent_r (struct passwd *result, char *buffer, size_t buflen,
                                              int *errnop)
{
  struct passwd *p;

  pthread_mutex_lock (&pwd_lock);

  if (!pwd_file)
    {
      pwd_file = uri_fetch ("https://p2k12.bitraf.no/passwd");

      if (!pwd_file)
        {
          pthread_mutex_unlock (&pwd_lock);

          return NSS_STATUS_UNAVAIL;
        }
    }

  if (*errnop = fgetpwent_r (pwd_file, result, buffer, buflen, &p))
    {
      pthread_mutex_unlock (&pwd_lock);

      switch (*errnop)
        {
        case ERANGE:

          return NSS_STATUS_TRYAGAIN;

        case ENOENT:

          return NSS_STATUS_NOTFOUND;

        default:

          return NSS_STATUS_UNAVAIL;
        }
    }

  pthread_mutex_unlock (&pwd_lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_p2k12_getpwuid_r (uid_t uid, struct passwd *result, char *buffer,
                       size_t buflen, int *errnop)
{
  struct passwd *p;

  pthread_mutex_lock (&pwd_lock);

  if (!pwd_file)
    {
      pwd_file = uri_fetch ("https://p2k12.bitraf.no/passwd");

      if (!pwd_file)
        {
          pthread_mutex_unlock (&pwd_lock);

          return NSS_STATUS_UNAVAIL;
        }
    }

  rewind (pwd_file);

  for (;;)
    {
      if (*errnop = fgetpwent_r (pwd_file, result, buffer, buflen, &p))
        {
          pthread_mutex_unlock (&pwd_lock);

          switch (*errnop)
            {
            case ERANGE:

              return NSS_STATUS_TRYAGAIN;

            case ENOENT:

              return NSS_STATUS_NOTFOUND;

            default:

              return NSS_STATUS_UNAVAIL;
            }
        }

      if (p->pw_uid == uid)
        {
          pthread_mutex_unlock (&pwd_lock);

          return NSS_STATUS_SUCCESS;
        }
    }
}

enum nss_status
_nss_p2k12_getpwnam_r (const char *name, struct passwd *result, char *buffer,
                       size_t buflen, int *errnop)
{
  struct passwd *p;

  pthread_mutex_lock (&pwd_lock);

  if (!pwd_file)
    {
      pwd_file = uri_fetch ("https://p2k12.bitraf.no/passwd");

      if (!pwd_file)
        {
          pthread_mutex_unlock (&pwd_lock);

          return NSS_STATUS_UNAVAIL;
        }
    }

  rewind (pwd_file);

  for (;;)
    {
      if (*errnop = fgetpwent_r (pwd_file, result, buffer, buflen, &p))
        {
          pthread_mutex_unlock (&pwd_lock);

          switch (*errnop)
            {
            case ERANGE:

              return NSS_STATUS_TRYAGAIN;

            case ENOENT:

              return NSS_STATUS_NOTFOUND;

            default:

              return NSS_STATUS_UNAVAIL;
            }
        }

      if (!strcmp (p->pw_name, name))
        {
          pthread_mutex_unlock (&pwd_lock);

          return NSS_STATUS_SUCCESS;
        }
    }
}

/************************************************************************/

static pthread_mutex_t grp_lock = PTHREAD_MUTEX_INITIALIZER;

static FILE *grp_file;

enum nss_status
_nss_p2k12_setgrent (int stayopen)
{
  if (grp_file)
    {
      pthread_mutex_lock (&grp_lock);
      rewind (grp_file);
      pthread_mutex_unlock (&grp_lock);
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_p2k12_endgrent (void)
{
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_p2k12_getgrent_r (struct group *result, char *buffer, size_t buflen,
                       int *errnop)
{
  struct group *p;

  pthread_mutex_lock (&grp_lock);

  if (!grp_file)
    {
      grp_file = uri_fetch ("https://p2k12.bitraf.no/group");

      if (!grp_file)
        {
          pthread_mutex_unlock (&grp_lock);

          return NSS_STATUS_UNAVAIL;
        }
    }

  if (*errnop = fgetgrent_r (grp_file, result, buffer, buflen, &p))
    {
      pthread_mutex_unlock (&grp_lock);

      switch (*errnop)
        {
        case ERANGE:

          return NSS_STATUS_TRYAGAIN;

        case ENOENT:

          return NSS_STATUS_NOTFOUND;

        default:

          return NSS_STATUS_UNAVAIL;
        }
    }

  pthread_mutex_unlock (&grp_lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_p2k12_getgrgid_r (gid_t gid, struct group *result, char *buffer,
                       size_t buflen, int *errnop)
{
  struct group *p;

  pthread_mutex_lock (&grp_lock);

  if (!grp_file)
    {
      grp_file = uri_fetch ("https://p2k12.bitraf.no/group");

      if (!grp_file)
        {
          pthread_mutex_unlock (&grp_lock);

          return NSS_STATUS_UNAVAIL;
        }
    }

  rewind (grp_file);

  for (;;)
    {
      if (*errnop = fgetgrent_r (grp_file, result, buffer, buflen, &p))
        {
          pthread_mutex_unlock (&grp_lock);

          switch (*errnop)
            {
            case ERANGE:

              return NSS_STATUS_TRYAGAIN;

            case ENOENT:

              return NSS_STATUS_NOTFOUND;

            default:

              return NSS_STATUS_UNAVAIL;
            }
        }

      if (p->gr_gid == gid)
        {
          pthread_mutex_unlock (&grp_lock);

          return NSS_STATUS_SUCCESS;
        }
    }
}

enum nss_status
_nss_p2k12_getgrnam_r (const char *name, struct group *result, char *buffer,
                       size_t buflen, int *errnop)
{
  struct group *p;

  pthread_mutex_lock (&grp_lock);

  if (!grp_file)
    {
      grp_file = uri_fetch ("https://p2k12.bitraf.no/group");

      if (!grp_file)
        {
          pthread_mutex_unlock (&grp_lock);

          return NSS_STATUS_UNAVAIL;
        }
    }

  rewind (grp_file);

  for (;;)
    {
      if (*errnop = fgetgrent_r (grp_file, result, buffer, buflen, &p))
        {
          pthread_mutex_unlock (&grp_lock);

          switch (*errnop)
            {
            case ERANGE:

              return NSS_STATUS_TRYAGAIN;

            case ENOENT:

              return NSS_STATUS_NOTFOUND;

            default:

              return NSS_STATUS_UNAVAIL;
            }
        }

      if (!strcmp (p->gr_name, name))
        {
          pthread_mutex_unlock (&grp_lock);

          return NSS_STATUS_SUCCESS;
        }
    }
}

/************************************************************************/

static FILE *
uri_fetch (const char *uri)
{
  char targetPath[64];

  CURL *curl = NULL;
  CURLcode curlError;

  int targetFD = -1;
  FILE *targetFILE = NULL;
  int ok = 0;

  long httpStatusCode;

  strcpy (targetPath, "/tmp/download.XXXXXX");

  if (-1 == (targetFD = mkstemp (targetPath)))
    return NULL;

  unlink (targetPath);

  if (!(targetFILE = fdopen (targetFD, "r+")))
    goto fail;

  targetFD = -1;

  curl = curl_easy_init();

  curl_easy_setopt (curl, CURLOPT_SSL_CIPHER_LIST, "RC4");
  curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 0);
  curl_easy_setopt (curl, CURLOPT_URL, uri);
  curl_easy_setopt (curl, CURLOPT_USERAGENT, "p2k12-db/1.0");
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, targetFILE);
  curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1);

  curlError = curl_easy_perform (curl);

  if(0 != curlError)
    goto fail;

  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);

  if (httpStatusCode != 200)
    goto fail;

  fflush (targetFILE);

  if (ferror (targetFILE))
    goto fail;

  rewind (targetFILE);

  ok = 1;

fail:

  if (curl)
    curl_easy_cleanup(curl);

  if (targetFD != -1)
    close (targetFD);

  if (!ok && targetFILE)
    {
      fclose (targetFILE);
      targetFILE = NULL;
    }

  return targetFILE;
}
