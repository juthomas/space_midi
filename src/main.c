#include "../inc/midi.h"


void ecrire_piste2(FILE *file)
{
	uint32_t mark = MIDI_write_track_header(file);
	
	MIDI_Instrument_Change(file, 0, 90) ;
	for(int i=C3 ; i<=C3+12 ; i=i+1)
	{
		MIDI_only_one_note_with_duration(file, 1, i, 64, QUARTER) ;        
	}
	MIDI_delta_time(file, 0) ;
	MIDI_Note(file, ON, 1, C3, 64) ;

	MIDI_delta_time(file, 0) ;
	MIDI_Note(file, ON, 1, C3+2, 64);

	MIDI_delta_time(file, QUARTER*2) ;
	MIDI_Note(file, OFF, 1, C3, 0) ;
	MIDI_delta_time(file, QUARTER*2) ;
	MIDI_Note(file, OFF, 1, C3+2, 0) ;
	// for(int i=0 ; i<=127 ; i=i+1){
	// 	MIDI_Program_Change(file, 0, i) ;
	// 	Note_unique_avec_duree(file, 0, C3 + 9, 64, noire) ;        
	// }

	MIDI_write_end_of_track(file) ;
	MIDI_write_track_lengh(file, mark);
}


void	midi_test(char *filename)
{
	printf("Hello World !\n");
	FILE *fichier_midi = fopen(filename, "wb") ;
	MIDI_write_file_header(fichier_midi, 1, 2, QUARTER) ;
	//metadatas
	MIDI_write_metadata(fichier_midi, 500000);
	//musique
	ecrire_piste2(fichier_midi) ;
	fclose(fichier_midi) ; 
}

void	midi_setup_file(char *filename, t_music_data *music_data)
{
	music_data->midi_file = fopen(filename, "wb") ;
	MIDI_write_file_header(music_data->midi_file, 1, 2, QUARTER) ;
	//metadatas
	MIDI_write_metadata(music_data->midi_file, music_data->quarter_value);
	music_data->midi_mark = MIDI_write_track_header(music_data->midi_file);
	MIDI_Instrument_Change(music_data->midi_file, 0, 90) ;

}

void	midi_write_measure_note(t_music_data *music_data, unsigned char state, \
		unsigned char channel, unsigned char note, unsigned char velocity)
{
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, state, channel, note, velocity) ;

}

void	midi_write_measure(t_server_data *server_data, \
		t_music_data *music_data, uint32_t measures_to_write)
{
	// T = 1/4
	//Code part
	midi_write_measure_note(music_data, ON, 1, C4, 64);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64) ;

	// T = 2/4
	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	//Code part
	midi_write_measure_note(music_data, OFF, 1, C4, 0);
	if (server_data->temperature > 0)
	{
		midi_write_measure_note(music_data, ON, 1,
		A0 + (server_data->temperature/3), server_data->light);
	}

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// T = 3/4

	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	//Code part
	if (server_data->temperature > 0)
	{
		midi_write_measure_note(music_data, OFF, 1, A0 + (server_data->temperature/3), 0);
	}
	if (server_data->motors_activity > 0)
	{
		midi_write_measure_note(music_data, ON, 1,
		A0 + (server_data->motors_activity/3), server_data->vibrations);
	}

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// T = 4/4
	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	//Code part
	if (server_data->motors_activity > 0)
	{
		midi_write_measure_note(music_data, OFF, 1, A0 + (server_data->motors_activity/3), 0);
	}

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// T = end
	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	// MIDI_delta_time(music_data->midi_file, 0) ;
	// MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// MIDI_Note(music_data->midi_file, OFF, 1, C3, 0) ;

}

void	midi_write_end(t_music_data *music_data)
{
	MIDI_write_end_of_track(music_data->midi_file);
	MIDI_write_track_lengh(music_data->midi_file, music_data->midi_mark);
	fclose(music_data->midi_file); 
}

uint32_t get_delta_time(struct timeval old, struct timeval recent)
{
	return ((recent.tv_sec - old.tv_sec) * 1000000 \
		+ (recent.tv_usec - old.tv_usec));
}

uint32_t get_time_to_measures(t_music_data music_data, \
			 struct timeval current_time)
{
	return ((uint32_t)(get_delta_time(music_data.entry_time, current_time) \
			/ music_data.measure_value));
}

uint32_t get_measures_to_write(t_music_data music_data, \
			 struct timeval current_time)
{
	return (get_time_to_measures(music_data, current_time) \
			- music_data.measures_writed);
}

int main(int argc, char **argv)
{
	t_server_data server_data = {.is_setup = 0, .sockfd = 0, .light = 0, .temperature = 0};
	t_music_data music_data = {.partition_duration=40000000,
	.measure_value=500000 * 4 * 2,
	.measures_writed=0,
	.quarter_value=500000 * 2
	};

	tcp_connect(&server_data);
	wait_for_connection(&server_data);
	midi_setup_file(argc == 2 ? argv[1] : "output.mid", &music_data);


	gettimeofday(&music_data.entry_time ,NULL);
	music_data.last_measure = music_data.entry_time;

	while (get_delta_time(music_data.entry_time, music_data.last_measure) \
		< music_data.partition_duration)
	{
		//Appel pour voir les valeurs du serv
		while (!tcp_get_fresh_data(&server_data))
		{
			wait_for_connection(&server_data);
		}

		struct timeval tv_tmp;
		gettimeofday(&tv_tmp, NULL);
		uint32_t diff_value = get_delta_time(music_data.last_measure, tv_tmp);

		music_data.last_measure = tv_tmp;
		//Rattraper les mesures de retard si besoin
		//Attendre le temps restant
		printf("data = t:%d, l:%d\n",server_data.temperature, server_data.light);


		printf("Diff : %u\n", diff_value);
		printf("Musures ecrites : %d\n", music_data.measures_writed);
		printf("Mesures a ecrire : %d\n", get_measures_to_write(music_data, tv_tmp));
		if (get_measures_to_write(music_data, tv_tmp))
		{
			//call midi write measure
			midi_write_measure(&server_data, &music_data, get_measures_to_write(music_data, tv_tmp));
			music_data.measures_writed += get_measures_to_write(music_data, tv_tmp);
		}

		usleep(500000);
	}

	midi_write_end(&music_data);
	close(server_data.sockfd);

	// tcp_connection(&server_data);

	// midi_test(argc == 2 ? argv[1] : "output.mid");
}