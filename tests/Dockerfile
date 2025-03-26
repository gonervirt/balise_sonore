FROM python:3.9-slim as backend
WORKDIR /app
COPY ./tests/mock_api.py .
COPY ./tests/requirements.txt .
RUN pip install -r requirements.txt
CMD ["python", "mock_api.py"]

FROM nginx:alpine as frontend
COPY ./data/www /usr/share/nginx/html
COPY ./tests/nginx.conf /etc/nginx/nginx.conf
EXPOSE 80
