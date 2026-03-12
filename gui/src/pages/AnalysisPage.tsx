import { useNavigate } from 'react-router-dom'
import { Button } from '@/components/ui/button'
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs'
import { Badge } from '@/components/ui/badge'
import { useStore } from '@/lib/store'
import ProgressIndicator from '@/components/ProgressIndicator'
import SetlistBarChart from '@/components/SetlistBarChart'
import SongCard from '@/components/SongCard'
import LoudnessChart from '@/components/LoudnessChart'
import BalanceChart from '@/components/BalanceChart'
import SectionAnalysis from '@/components/SectionAnalysis'
import RecommendationsList from '@/components/RecommendationsList'
import ExportPanel from '@/components/ExportPanel'
import { useEffect } from 'react'

export default function AnalysisPage() {
  const navigate = useNavigate()
  const {
    status,
    results,
    error,
    selectedSongId,
    selectSong,
    viewMode,
    setViewMode,
  } = useStore()

  const selectedSong = results?.songs.find((s) => s.song_id === selectedSongId)

  useEffect(() => {
    if (!window.electronAPI) return
    const unsubscribe = window.electronAPI.onProgress((data) => {
      useStore.getState().setProgress(data)
    })
    return () => { unsubscribe() }
  }, [])

  const gradeColor = (grade: string) => {
    switch (grade) {
      case 'Good':
        return 'bg-chart-1/10 text-chart-1 border-chart-1/20'
      case 'Fair':
        return 'bg-chart-4/10 text-chart-4 border-chart-4/20'
      default:
        return 'bg-destructive/10 text-destructive border-destructive/20'
    }
  }

  return (
    <div className="flex flex-col h-screen">
      <header className="border-b px-6 py-3 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <Button variant="ghost" size="sm" onClick={() => navigate('/')}>
            &larr; Home
          </Button>
          <span className="font-semibold">
            {results?.project_name ?? 'Analysis'}
          </span>
          {results?.setlist && (
            <Badge variant="outline" className={gradeColor(results.setlist.grade)}>
              {results.setlist.grade}
            </Badge>
          )}
        </div>
        <div className="flex items-center gap-2">
          <Tabs value={viewMode} onValueChange={(v) => setViewMode(v as 'beginner' | 'advanced')}>
            <TabsList className="h-8">
              <TabsTrigger value="beginner" className="text-xs px-3">Simple</TabsTrigger>
              <TabsTrigger value="advanced" className="text-xs px-3">Advanced</TabsTrigger>
            </TabsList>
          </Tabs>
        </div>
      </header>

      <div className="flex-1 overflow-y-auto p-6 space-y-6">
        {status === 'running' && <ProgressIndicator />}

        {status === 'error' && (
          <div className="text-center py-12">
            <p className="text-destructive text-lg mb-2">Analysis failed</p>
            {error && <p className="text-sm text-muted-foreground mb-4 max-w-md mx-auto">{error}</p>}
            <Button onClick={() => navigate('/')}>Go Home</Button>
          </div>
        )}

        {status === 'complete' && results && (
          <>
            {/* Setlist overview */}
            <section className="space-y-4">
              <div className="flex items-center justify-between">
                <h2 className="text-xl font-semibold">Setlist Overview</h2>
                {viewMode === 'advanced' && (
                  <div className="text-sm text-muted-foreground space-x-4">
                    <span>Median: {results.setlist.median_lufs?.toFixed(1)} LUFS</span>
                    <span>Std Dev: {results.setlist.lufs_stddev?.toFixed(1)} dB</span>
                    <span>Consistency: {(results.setlist.consistency_score * 100).toFixed(0)}%</span>
                  </div>
                )}
              </div>

              <SetlistBarChart songs={results.songs} medianLufs={results.setlist.median_lufs} />

              {results.setlist.recommendations.length > 0 && (
                <RecommendationsList
                  recommendations={results.setlist.recommendations}
                  level="setlist"
                />
              )}
            </section>

            {/* Song tabs */}
            {results.songs.length > 0 && (
              <section>
                <Tabs
                  value={selectedSongId ?? results.songs[0].song_id}
                  onValueChange={selectSong}
                >
                  <TabsList className="flex-wrap h-auto gap-1">
                    {results.songs.map((song) => (
                      <TabsTrigger key={song.song_id} value={song.song_id}>
                        {song.song_name}
                        <Badge
                          variant="outline"
                          className={`ml-2 text-xs ${gradeColor(song.scores.grade)}`}
                        >
                          {song.scores.grade}
                        </Badge>
                      </TabsTrigger>
                    ))}
                  </TabsList>

                  {results.songs.map((song) => (
                    <TabsContent key={song.song_id} value={song.song_id} className="space-y-6 mt-4">
                      <SongCard song={song} viewMode={viewMode} />

                      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                        <LoudnessChart song={song} viewMode={viewMode} />
                        <BalanceChart song={song} viewMode={viewMode} />
                      </div>

                      {viewMode === 'advanced' && (
                        <SectionAnalysis song={song} />
                      )}

                      <RecommendationsList
                        recommendations={song.recommendations}
                        level="song"
                      />

                      <ExportPanel />
                    </TabsContent>
                  ))}
                </Tabs>
              </section>
            )}
          </>
        )}

        {status === 'idle' && (
          <div className="text-center py-12">
            <p className="text-muted-foreground mb-4">No analysis results yet.</p>
            <Button onClick={() => navigate('/')}>Go Home</Button>
          </div>
        )}
      </div>
    </div>
  )
}
