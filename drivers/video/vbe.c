#include <stdint.h>
#include <drv/vbe.h>

void pixel(struct multiboot_info* mboot, int x, int y, uint8_t color)
{
    unsigned char *pixel = mboot->framebuffer_addr + y*mboot->framebuffer_pitch + x;
    *pixel = color;
}

// Function to print a character at a specific position with a color attribute
void print_char(struct multiboot_info* boot_info, char c, int x, int y, uint16_t color) {
    if (c < 0 || c > 127) return; // Check for valid ASCII range

    // Get font data for the character
    const uint8_t *char_data = font[(uint8_t)c];

    // Draw the character on the screen
    for (int row = 0; row < FONT_HEIGHT; row++) {
        for (int col = 0; col < FONT_WIDTH; col++) {
            // Check if the bit for the current pixel is set
            if (char_data[row] & (1 << col)) {
                // Set the pixel color based on the provided color attribute
                pixel(boot_info, x + col, y + row, color); // Draw the pixel
            }
        }
    }
}

// Example usage of print_char with color attributes
void display_text(struct multiboot_info* boot_info, const char *text, int start_x, int start_y, uint16_t color) {
    int x = start_x;
    int y = start_y;

    while (*text) 
    {
        if (*text == '\n') 
        { 
            x = 0; 
            y += FONT_HEIGHT; // Move to the next line
            continue;
        }
        else
        {
            print_char(boot_info, *text, x, y, color); // Print character with color
            x += FONT_WIDTH; // Move to the next character position
            if (x >= boot_info->framebuffer_width) { // If reached the end of the line
                x = start_x; // Reset x
                y += FONT_HEIGHT; // Move to the next line
            }
            text++;
        }
    }
}

void vbe_printf(struct multiboot_info* boot_info, char *text, int x, int y, uint16_t color)
{
    int posX = x * 8;
    int posY = y * 8;
    display_text(boot_info, text, posX, posY, color);
}