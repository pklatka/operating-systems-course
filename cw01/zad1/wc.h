typedef struct wc_data
{
    char **data;
    int max_block_size;
    int current_block_size;
} wc_data;

wc_data wc_init(int max_block_size);

int wc_start(struct wc_data *wc, char *filename);

char * wc_get(struct wc_data *wc, int index);

int wc_free(struct wc_data *wc, int index);

int wc_destroy(struct wc_data *wc);
