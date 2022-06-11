
int main()
{
    SSL_library_init();
    SSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_NEW(TLS_client_method());

    SSL *ssl = SSL_NEW(ctx);

    SSL_set_tlsext_host_name(ssl, hostname);
    SSL_set_fd(ssl, server);

    SSL_connect(ssl);

    SSL_write(ssl, buff, strlen(buff));

    while (1)
    {
        int rbytes = SSL_read(ssl, buff, sizeof(buff));
        if(rbytes < 1 )
            printf("closed");
        printf("%.*s", rbytes, buff);
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
}