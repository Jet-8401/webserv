# Dockerfile for MariaDB
FROM debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive

ARG MYSQL_USER
ARG MYSQL_DATABASE

# update syatem and installing mariadb then starting it
RUN --mount=type=secret,id=db_password,required=true \
    --mount=type=secret,id=db_root_password,required=true \
    apt-get update && apt-get upgrade -y && apt-get install -y --no-install-recommends mariadb-server \
    && service mariadb start \
    && mysql -e "CREATE DATABASE IF NOT EXISTS ${MYSQL_DATABASE};" \
    && mysql -e "CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '$(cat /run/secrets/db_password)';" \
    && mysql -e "GRANT ALL PRIVILEGES ON ${MYSQL_DATABASE}.* TO '${MYSQL_USER}'@'%';" \
    && mysql -e "FLUSH PRIVILEGES;" \
    && mysql -e "ALTER USER 'root'@'localhost' IDENTIFIED BY '$(cat /run/secrets/db_root_password)';" \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Start the MariaDB daemon as the entrypoint
ENTRYPOINT ["/usr/sbin/mysqld"]

# Set default arguments for MariaDB
CMD ["--bind-address", "0.0.0.0"]

