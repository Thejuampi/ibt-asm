%defstr artifact SIZE_CHECK_FILE
incbin artifact
times ((($ - $$) <= APP_FILE_LIMIT) - 1) db 0
