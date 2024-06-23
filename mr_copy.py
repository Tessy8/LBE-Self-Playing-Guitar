import mido

# Standard tuning E-A-D-G-B-E'
guitar_tuning = [40, 45, 50, 55, 59, 64]  # MIDI note numbers for open strings

def parse_midi_file(file_path):
    midi = mido.MidiFile(file_path)
    ticks_per_beat = midi.ticks_per_beat

    notes_with_timing = []
    track_times = [0] * len(midi.tracks)
    current_tempo = 500000  # Default tempo (120 BPM)
    
    for i, track in enumerate(midi.tracks):
        for msg in track:
            track_times[i] += msg.time
            if msg.type == 'set_tempo':
                current_tempo = msg.tempo
            if msg.type == 'note_on' and msg.velocity > 0:
                current_time = track_times[i]
                time_in_seconds = mido.tick2second(current_time, ticks_per_beat, current_tempo)
                notes_with_timing.append((msg.note, current_time, time_in_seconds))
    
    return sorted(notes_with_timing, key=lambda x: x[1])

def map_note_to_guitar(note):
    positions = []
    for string, open_note in enumerate(guitar_tuning):
        fret = note - open_note
        if 0 <= fret <= 9:  # Assuming 9 playable frets on the guitar
            positions.append((string + 1, fret))
    return positions


midi_file_path = "MIDI files\Go-Tell-It-On-The-Mountain.mid"
notes_with_timing = parse_midi_file(midi_file_path)

for note, ticks, time_in_seconds in notes_with_timing:
    positions = map_note_to_guitar(note)
    for string, fret in positions:
        print(f'Note: {note}, Time (ticks): {ticks}, Time (seconds): {time_in_seconds:.2f}, String: {string}, Fret: {fret}')
