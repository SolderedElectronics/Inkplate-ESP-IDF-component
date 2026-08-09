/**
 * @file        webserver.h
 * @author      Fran Fodor for Soldered
 * @brief       HTTP route declarations for the LAN gallery web server.
 *
 * @details     Declares the entry point used by main.cpp to start the
 *              esp_http_server instance and register the gallery routes
 *              implemented in webserver.cpp ("/" and "/upload").
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

/**
 * @brief Starts the HTTP server and registers the gallery routes.
 *
 * @note Registers:
 *       - GET  "/"       -> serves the upload page (main/html.h).
 *       - POST "/upload" -> receives an uploaded image and writes it to
 *         the SD card via startFileUpload()/writeFileData()/
 *         finishFileUpload() (implemented in main.cpp).
 */
void setupWebServer();
