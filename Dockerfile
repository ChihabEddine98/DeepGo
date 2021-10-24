FROM python:3.8-slim
FROM tensorflow/tensorflow

COPY ./src  ./app/src
COPY ./utils ./app/utils
COPY ./requirements.txt ./app/requirements.txt

WORKDIR /app

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    python3-dev \
    python3-setuptools 

RUN pip install --ignore-installed  -r requirements.txt 