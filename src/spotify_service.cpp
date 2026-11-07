#include "spotify_service.h"
#include "config.h"
#include <ArduinoJson.h>
#include <TJpg_Decoder.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

SpotifyService::SpotifyService() : _tokenExpiresAt(0) {}

bool SpotifyService::sendTokenRequest(String &newToken, uint32_t &expiresIn) {
#if defined(SPOTIFY_ENABLED) && SPOTIFY_ENABLED
    if (WiFi.status() != WL_CONNECTED) return false;

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    if (!client) return false;
    client->setInsecure();
    client->setBufferSizes(1024, 512);

    HTTPClient https;
    https.setTimeout(3000);
    if (!https.begin(*client, "https://accounts.spotify.com/api/token")) {
        return false;
    }

    https.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "grant_type=refresh_token&refresh_token=";
    postData += SPOTIFY_REFRESH_TOKEN;
    postData += "&client_id=";
    postData += SPOTIFY_CLIENT_ID;
    postData += "&client_secret=";
    postData += SPOTIFY_CLIENT_SECRET;

    int httpCode = https.POST(postData);
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[Spotify] Error renovando token. HTTP: %d\n", httpCode);
        https.end();
        return false;
    }

    WiFiClient *stream = https.getStreamPtr();
    JsonDocument filter;
    filter["access_token"] = true;
    filter["expires_in"] = true;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, *stream, DeserializationOption::Filter(filter));
    https.end();

    if (err) {
        Serial.printf("[Spotify] Error parseando respuesta token: %s\n", err.c_str());
        return false;
    }

    newToken = doc["access_token"].as<String>();
    expiresIn = doc["expires_in"] | 3600;
    return (newToken.length() > 0);
#else
    return false;
#endif
}

bool SpotifyService::refreshAccessToken() {
    Serial.println(F("[Spotify] Renovando Access Token..."));
    String newToken;
    uint32_t expiresIn = 3600;

    if (sendTokenRequest(newToken, expiresIn)) {
        _accessToken = newToken;
        _tokenExpiresAt = millis() + ((expiresIn > 120 ? expiresIn - 120 : expiresIn) * 1000UL);
        Serial.println(F("[Spotify] Access Token obtenido OK."));
        return true;
    }

    return false;
}

bool SpotifyService::getCurrentlyPlaying(SpotifyTrack &track) {
#if defined(SPOTIFY_ENABLED) && SPOTIFY_ENABLED
    if (WiFi.status() != WL_CONNECTED) return false;

    if (!hasValidToken() || millis() >= _tokenExpiresAt) {
        if (!refreshAccessToken()) {
            return false;
        }
    }

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    if (!client) return false;
    client->setInsecure();
    client->setBufferSizes(2048, 512);

    HTTPClient https;
    https.setTimeout(3000);
    if (!https.begin(*client, "https://api.spotify.com/v1/me/player/currently-playing")) {
        return false;
    }

    String authHeader = "Bearer " + _accessToken;
    https.addHeader("Authorization", authHeader.c_str());

    int httpCode = https.GET();

    // 204 No Content: No hay reproducción activa
    if (httpCode == 204) {
        track.isPlaying = false;
        track.hasData = false;
        https.end();
        return true;
    }

    if (httpCode != HTTP_CODE_OK) {
        if (httpCode == 401) {
            _accessToken = "";
        }
        https.end();
        return false;
    }

    JsonDocument filter;
    filter["is_playing"] = true;
    filter["progress_ms"] = true;
    filter["item"]["id"] = true;
    filter["item"]["name"] = true;
    filter["item"]["duration_ms"] = true;
    filter["item"]["artists"][0]["name"] = true;
    filter["item"]["album"]["name"] = true;
    filter["item"]["album"]["images"][0]["url"] = true;
    filter["item"]["album"]["images"][1]["url"] = true;
    filter["item"]["album"]["images"][2]["url"] = true;

    WiFiClient *stream = https.getStreamPtr();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, *stream, DeserializationOption::Filter(filter));
    https.end();

    if (err) {
        Serial.printf("[Spotify] Error parseando JSON: %s\n", err.c_str());
        return false;
    }

    track.hasData = true;
    track.isPlaying = doc["is_playing"] | false;
    track.progressMs = doc["progress_ms"] | 0;

    JsonObject item = doc["item"];
    if (!item.isNull()) {
        track.id = item["id"].as<String>();
        track.title = item["name"].as<String>();
        track.durationMs = item["duration_ms"] | 0;

        JsonArray artists = item["artists"];
        if (artists.size() > 0) {
            track.artist = artists[0]["name"].as<String>();
        }

        JsonArray images = item["album"]["images"];
        if (images.size() > 0) {
            // Último elemento: miniatura pequeña (~64x64)
            int lastIdx = images.size() - 1;
            track.imageUrl = images[lastIdx]["url"].as<String>();
        }
    }

    return true;
#else
    return false;
#endif
}

bool SpotifyService::downloadAlbumArt(const String &url, int16_t x, int16_t y) {
#if defined(SPOTIFY_ENABLED) && SPOTIFY_ENABLED
    if (url.length() == 0 || WiFi.status() != WL_CONNECTED) return false;

    // Salvaguarda de memoria: evitar alocar si el heap es demasiado bajo
    if (ESP.getFreeHeap() < 13000) {
        Serial.println(F("[Spotify] Heap bajo, omitiendo descarga de carátula"));
        return false;
    }

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    if (!client) return false;
    client->setInsecure();
    client->setBufferSizes(2048, 512);

    HTTPClient https;
    https.setTimeout(3000);
    if (!https.begin(*client, url)) {
        return false;
    }

    int httpCode = https.GET();
    if (httpCode != HTTP_CODE_OK) {
        https.end();
        return false;
    }

    int len = https.getSize();
    if (len <= 0 || len > 12000) { // Miniaturas 64x64 rara vez superan 6 KB
        https.end();
        return false;
    }

    WiFiClient *stream = https.getStreamPtr();
    uint8_t *jpgBuffer = (uint8_t *)malloc(len);
    if (!jpgBuffer) {
        Serial.println(F("[Spotify] No hay RAM contigua para buffer JPEG"));
        https.end();
        return false;
    }

    int bytesRead = stream->readBytes(jpgBuffer, len);
    https.end();

    if (bytesRead == len) {
        TJpgDec.drawJpg(x, y, jpgBuffer, len);
        free(jpgBuffer);
        return true;
    }

    free(jpgBuffer);
    return false;
#else
    return false;
#endif
}

bool SpotifyService::sendPlayerCommand(const char* endpoint, const char* method) {
#if defined(SPOTIFY_ENABLED) && SPOTIFY_ENABLED
    if (!hasValidToken() || WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[Spotify] Error: Sin token valido o sin conexion"));
        return false;
    }

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    if (!client) return false;
    client->setInsecure();
    client->setBufferSizes(1024, 512);

    HTTPClient https;
    https.setTimeout(3000);
    String url = String("https://api.spotify.com/v1/me/player/") + endpoint;
    if (!https.begin(*client, url)) {
        Serial.println(F("[Spotify] Error al iniciar conexion HTTPS"));
        return false;
    }

    https.addHeader("Authorization", "Bearer " + _accessToken);
    https.addHeader("Content-Length", "0");

    int httpCode = 0;
    if (strcmp(method, "PUT") == 0) {
        httpCode = https.sendRequest("PUT", (String)"");
    } else if (strcmp(method, "POST") == 0) {
        httpCode = https.POST("");
    }

    String resp = https.getString();
    https.end();

    Serial.printf("[Spotify] Comando %s %s -> HTTP: %d | Resp: %s\n", method, endpoint, httpCode, resp.c_str());

    if (httpCode == 403) {
        Serial.println(F("[Spotify] AVISO 403: Se requiere cuenta Spotify Premium o falta el permiso 'user-modify-playback-state' en el token"));
    } else if (httpCode == 404) {
        Serial.println(F("[Spotify] AVISO 404: No hay un dispositivo activo reproduciendo Spotify en este momento"));
    }

    return (httpCode == 204 || httpCode == 200);
#else
    return false;
#endif
}

bool SpotifyService::togglePlayPause(bool currentlyPlaying) {
    if (currentlyPlaying) {
        return sendPlayerCommand("pause", "PUT");
    } else {
        return sendPlayerCommand("play", "PUT");
    }
}

bool SpotifyService::nextTrack() {
    return sendPlayerCommand("next", "POST");
}

bool SpotifyService::previousTrack() {
    return sendPlayerCommand("previous", "POST");
}
