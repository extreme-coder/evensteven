import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ReferenceLine,
  ResponsiveContainer,
  Legend,
} from 'recharts'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import type { SongAnalysis } from '@/lib/store'

interface Props {
  song: SongAnalysis
  viewMode: 'beginner' | 'advanced'
}

function formatTime(seconds: number) {
  const m = Math.floor(seconds / 60)
  const s = Math.floor(seconds % 60)
  return `${m}:${s.toString().padStart(2, '0')}`
}

export default function LoudnessChart({ song, viewMode }: Props) {
  const data = song.loudness.timestamps.map((t, i) => ({
    time: t,
    shortterm: song.loudness.shortterm_lufs[i] ?? null,
    momentary: viewMode === 'advanced' ? (song.loudness.momentary_lufs[i] ?? null) : null,
  }))

  // Downsample for performance if too many points
  const maxPoints = 500
  const step = Math.max(1, Math.floor(data.length / maxPoints))
  const sampled = data.filter((_, i) => i % step === 0)

  return (
    <Card>
      <CardHeader className="pb-2">
        <CardTitle className="text-base">Loudness Timeline</CardTitle>
      </CardHeader>
      <CardContent>
        <ResponsiveContainer width="100%" height={280}>
          <LineChart data={sampled} margin={{ top: 5, right: 20, bottom: 5, left: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="var(--border)" />
            <XAxis
              dataKey="time"
              tickFormatter={formatTime}
              type="number"
              domain={['dataMin', 'dataMax']}
              tick={{ fontSize: 11, fill: 'var(--muted-foreground)' }}
            />
            <YAxis
              tick={{ fontSize: 11, fill: 'var(--muted-foreground)' }}
              domain={['auto', 'auto']}
              label={{ value: 'LUFS', angle: -90, position: 'insideLeft', style: { fontSize: 11, fill: 'var(--muted-foreground)' } }}
            />
            <Tooltip
              labelFormatter={(v) => formatTime(Number(v))}
              formatter={(v: number) => [v?.toFixed(1) + ' LUFS', '']}
              contentStyle={{ backgroundColor: 'var(--popover)', border: '1px solid var(--border)', color: 'var(--popover-foreground)' }}
            />
            <Legend />
            {song.loudness.integrated_lufs != null && isFinite(song.loudness.integrated_lufs) && (
              <ReferenceLine
                y={song.loudness.integrated_lufs}
                stroke="var(--destructive)"
                strokeDasharray="5 5"
                label={{ value: `${song.loudness.integrated_lufs.toFixed(1)}`, position: 'right', fontSize: 10, fill: 'var(--destructive)' }}
              />
            )}
            <Line
              type="monotone"
              dataKey="shortterm"
              stroke="var(--chart-2)"
              dot={false}
              strokeWidth={1.5}
              name="Short-term"
            />
            {viewMode === 'advanced' && (
              <Line
                type="monotone"
                dataKey="momentary"
                stroke="var(--chart-3)"
                dot={false}
                strokeWidth={0.8}
                opacity={0.6}
                name="Momentary"
              />
            )}
          </LineChart>
        </ResponsiveContainer>
      </CardContent>
    </Card>
  )
}
