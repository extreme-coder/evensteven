import { create } from 'zustand'

export interface AudioMetadata {
  filename: string
  duration_s: number
  sample_rate: number
  channels: number
  format: string
}

export interface SongEntry {
  id: string
  name: string
  path: string
  metadata?: AudioMetadata
}

export interface LoudnessResult {
  integrated_lufs: number | null
  shortterm_lufs: number[]
  momentary_lufs: number[]
  rms_db: number | null
  peak_db: number | null
  dynamic_range_db: number
  band_energy: Record<string, number | null>
  timestamps: number[]
}

export interface BalanceResult {
  vocal_presence_score: number
  masking_risk_score: number
  vocal_energy_db: number[]
  accompaniment_energy_db: number[]
  balance_db: number[]
  masking_risk: boolean[]
  timestamps: number[]
}

export interface Section {
  label: string
  start_s: number
  end_s: number
  is_silence: boolean
}

export interface SongScore {
  loudness_consistency: number
  balance_score: number
  composite: number
  grade: string
}

export interface SongAnalysis {
  song_id: string
  song_name: string
  metadata: AudioMetadata
  loudness: LoudnessResult
  balance: BalanceResult
  sections: Section[]
  scores: SongScore
  recommendations: string[]
}

export interface SetlistScore {
  median_lufs: number
  lufs_stddev: number
  consistency_score: number
  grade: string
  recommendations: string[]
}

export interface AnalysisResults {
  project_id: string
  project_name: string
  timestamp: string
  setlist: SetlistScore
  songs: SongAnalysis[]
}

export interface ProgressInfo {
  stage: string
  percent: number
  song: string
}

type AnalysisStatus = 'idle' | 'running' | 'complete' | 'error'
type ViewMode = 'beginner' | 'advanced'

interface AppState {
  // Project
  projectName: string
  songs: SongEntry[]

  // Analysis
  status: AnalysisStatus
  progress: ProgressInfo | null
  results: AnalysisResults | null
  error: string | null

  // UI
  selectedSongId: string | null
  viewMode: ViewMode

  // Actions
  setProjectName: (name: string) => void
  addSongs: (songs: SongEntry[]) => void
  updateSongMetadata: (id: string, metadata: AudioMetadata) => void
  removeSong: (id: string) => void
  reorderSongs: (songs: SongEntry[]) => void
  setStatus: (status: AnalysisStatus) => void
  setProgress: (progress: ProgressInfo | null) => void
  setResults: (results: AnalysisResults | null) => void
  setError: (error: string | null) => void
  selectSong: (id: string | null) => void
  setViewMode: (mode: ViewMode) => void
  reset: () => void
}

const initialState = {
  projectName: 'Untitled Project',
  songs: [] as SongEntry[],
  status: 'idle' as AnalysisStatus,
  progress: null as ProgressInfo | null,
  results: null as AnalysisResults | null,
  error: null as string | null,
  selectedSongId: null as string | null,
  viewMode: 'beginner' as ViewMode,
}

export const useStore = create<AppState>((set) => ({
  ...initialState,

  setProjectName: (name) => set({ projectName: name }),

  addSongs: (newSongs) =>
    set((state) => ({ songs: [...state.songs, ...newSongs] })),

  updateSongMetadata: (id, metadata) =>
    set((state) => ({
      songs: state.songs.map((s) => (s.id === id ? { ...s, metadata } : s)),
    })),

  removeSong: (id) =>
    set((state) => ({
      songs: state.songs.filter((s) => s.id !== id),
    })),

  reorderSongs: (songs) => set({ songs }),

  setStatus: (status) => set({ status }),
  setProgress: (progress) => set({ progress }),
  setResults: (results) =>
    set({
      results,
      selectedSongId: results?.songs?.[0]?.song_id ?? null,
    }),
  setError: (error) => set({ error, status: 'error' }),

  selectSong: (id) => set({ selectedSongId: id }),
  setViewMode: (mode) => set({ viewMode: mode }),

  reset: () => set(initialState),
}))
