package main

import (
	"fmt"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

func isFileAllowed(file string) error {
	file = strings.TrimSpace(file)
	if file == "" {
		return fmt.Errorf("file name cannot be empty")
	}
	_, filename := filepath.Split(file)
	if strings.Contains(filename, "flag.txt") {
		return fmt.Errorf("access to flag.txt is not allowed")
	}
	return nil
}

func main() {
	var err error

	fmt.Println("Ready, Set, Go!")

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {

		if r.Method != http.MethodGet {
			http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
			return
		}

		filename := r.URL.Query().Get("file_name")

		err = isFileAllowed(filename)
		fmt.Printf("[DEBUG] is %s allowed? %t\n", filename, err == nil)
		if err != nil {
			http.Error(w, err.Error(), http.StatusForbidden)
			return
		}

		data, err := os.ReadFile(filename)
		if err != nil {
			http.Error(w, fmt.Sprintf("Error reading file: %s", err), http.StatusInternalServerError)
			return
		}
		fmt.Fprintf(w, "%s\n", data)
	})

	http.ListenAndServe(":8080", nil)
}
