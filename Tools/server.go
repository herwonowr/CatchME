package main

import (
	"log"
	"net/http"
)

func main() {
	fs := http.FileServer(http.Dir("../CatchME_UI/"))
	http.Handle("/", fs)

	log.Println("Starting Web Server...")
	log.Println("To stop Web Server: CTRL+C")
	log.Println("Listening on: http://127.0.0.1:3000")
	http.ListenAndServe(":3000", nil)
}
