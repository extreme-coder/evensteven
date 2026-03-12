import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import type { SongAnalysis } from '@/lib/store'

interface Props {
  song: SongAnalysis
  viewMode: 'beginner' | 'advanced'
}

export default function SongCard({ song, viewMode }: Props) {
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

  const fmt = (v: number | null, digits = 1) =>
    v != null && isFinite(v) ? v.toFixed(digits) : '—'

  return (
    <Card>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg">{song.song_name}</CardTitle>
          <Badge variant="outline" className={`text-sm ${gradeColor(song.scores.grade)}`}>
            {song.scores.grade}
          </Badge>
        </div>
      </CardHeader>
      <CardContent>
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
          <Metric
            label="Integrated LUFS"
            value={fmt(song.loudness.integrated_lufs)}
          />
          <Metric
            label="Vocal Presence"
            value={`${(song.balance.vocal_presence_score * 100).toFixed(0)}%`}
          />
          <Metric
            label="Masking Risk"
            value={`${(song.balance.masking_risk_score * 100).toFixed(0)}%`}
          />
          <Metric
            label="Score"
            value={`${(song.scores.composite * 100).toFixed(0)}%`}
          />
        </div>

        {viewMode === 'advanced' && (
          <div className="grid grid-cols-2 sm:grid-cols-4 gap-4 mt-3 pt-3 border-t">
            <Metric label="Peak" value={`${fmt(song.loudness.peak_db)} dB`} />
            <Metric label="RMS" value={`${fmt(song.loudness.rms_db)} dB`} />
            <Metric label="Dynamic Range" value={`${fmt(song.loudness.dynamic_range_db)} dB`} />
            <Metric
              label="Duration"
              value={`${Math.floor(song.metadata.duration_s / 60)}:${Math.floor(song.metadata.duration_s % 60).toString().padStart(2, '0')}`}
            />
          </div>
        )}
      </CardContent>
    </Card>
  )
}

function Metric({ label, value }: { label: string; value: string }) {
  return (
    <div>
      <p className="text-xs text-muted-foreground">{label}</p>
      <p className="text-lg font-semibold">{value}</p>
    </div>
  )
}
